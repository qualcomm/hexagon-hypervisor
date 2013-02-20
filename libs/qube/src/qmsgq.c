/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <qube.h>
#include <hexagon_protos.h>

#if 0
#define DEBUGMSG(...) blast_printf(__VA_ARGS__);
#else
#define DEBUGMSG(...) /* nothing */
#endif

#define N_SHORTBUFS 32

struct qmsgq_attach_node {
	struct qmsgq_attach_node *next;
	char *name;
	h2_cond_t lock;
};

//static blast_mutex_t lock = {0,QMSGQMUTEX_QUEUE_ID,0,0};
//static blast_mutex_t lock = {MUTEX_MAGIC,0,0,0};
static h2_mutex_t lock = H2_MUTEX_T_INIT;

static struct _qmsgq_node *head;
static struct qmsgq_attach_node *attach_list; /* Nodes attached before the qmsgq
                                               * is created, currently blocked */

typedef struct cacheline_struct {
	long long int dw0;
	long long int dw1;
	long long int dw2;
	long long int dw3;
} cacheline_t __attribute__((aligned(32)));

static cacheline_t shortbuf[N_SHORTBUFS];
static unsigned int shortbuf_valids = 0;

static inline unsigned int shortbuf_getbuf_lockless()
{
	unsigned int ret;
	asm (
	"1:\t r8 = memw_locked(%1) // get valid mask\n"
	"\t %0 = ct1(r8) // get first clear entry\n"
	"\t { r8 = setbit(r8,%0) // mark it as used\n"
	"\t   p0 = cmp.eq(%0,#32) // all used?\n"
	"\t   if (p0.new) jump:nt 2f } // yes, abort\n"
	"\t memw_locked(%1,p0) = r8 // store new mask\n"
	"\t if (!p0) jump 1b // not atomic\n"
	"2:\n" : "=&r"(ret) : "r"(&shortbuf_valids)
	:"r8","p0","memory");
	return ret;
}

static inline void shortbuf_clearbit_lockless(int whatbit)
{
	asm (
	"1:\t  r8 = memw_locked(%1) // get valid mask\n"
	"\t    r8 = clrbit(r8,%0) // clear appropriate bit\n"
	"\t    memw_locked(%1,p0) = r8 // store new mask\n"
	"\t if (!p0) jump 1b // not atomic\n"
	"\t \n" :: "r"(whatbit),"r"(&shortbuf_valids)
	:"r8","p0","memory");
}

void qmsgq_init(){
    //lock.queue = blast_futex_alloc_wait_queue();
}

int qmsgq_create(const char *name, qmsgq_t *msgq, qmsgq_attr_t *attr)
{
	struct qmsgq_attach_node *wait_node;
	struct _qmsgq_node *tmp;
	h2_pipe_t *ptmp;
	if ((tmp = h2_malloc(sizeof(*tmp))) == NULL) return EMEM;
	if ((ptmp = h2_malloc(sizeof(*ptmp))) == NULL) return EMEM;
	if (((unsigned int)attr->buf) & 7) return EVAL;
	memcpy(&tmp->attrs,attr,sizeof(*attr));
	tmp->name = name;
	DEBUGMSG("pipe create: ptmp=%x attr->buf=%x\n",ptmp,attr->buf);
	if ((tmp->blastpipe = h2_pipe_create(ptmp,attr->buf,attr->buf_size/8)) == NULL) {
		h2_printf("h2_pipe_create failed?!?\n");
	}
	*attr->channel = tmp;
	h2_mutex_lock(&lock);
	tmp->next = head;
	head = tmp;
	*msgq = tmp;
	DEBUGMSG("created msgq=%x\n",tmp);

	/* If there is an out standing attach on this msgq wake up the client */
	for (wait_node = attach_list; wait_node != NULL; wait_node = wait_node->next) {
		if (strcmp (wait_node->name, name) == 0) {
			//blast_mutex_unlock (&wait_node->lock);
			h2_cond_signal (&wait_node->lock);
			/* There could be multiple clients
			break;
			 */
		}
	}
	h2_mutex_unlock(&lock);
	return EOK;
}

int qmsgq_attach(const char *name, qmsgq_t *msgq)
{
	struct qmsgq_attach_node *wait_node = NULL;
	struct _qmsgq_node *tmp;
	*msgq = NULL;
	h2_mutex_lock(&lock);
	for (tmp = head; tmp != NULL; tmp = tmp->next) {
		if (0 == strcmp(tmp->name,name)) {
			*msgq = tmp;
			break;
		}
	}

	if (*msgq == NULL) {
		wait_node = (struct qmsgq_attach_node *)h2_malloc (sizeof (struct qmsgq_attach_node) + strlen (name) + 1);
		wait_node->name = (char *)(wait_node + 1);
		strcpy (wait_node->name, name);
		wait_node->next = attach_list;
		attach_list = wait_node;
		h2_cond_init (&wait_node->lock);

        while (1) {
            h2_cond_wait(&wait_node->lock, &lock);
            
            for (tmp = head; tmp != NULL; tmp = tmp->next) {
                if (0 == strcmp(tmp->name,name)) {
                    *msgq = tmp;
                    goto attach_success;
                }
            }
        }
	}
attach_success:
    h2_mutex_unlock(&lock);
	return EOK;
}

int qmsgq_delete(qmsgq_t msgq)
{
	struct _qmsgq_node *tmp;
	int found = 0;
	h2_mutex_lock(&lock);
	if (head == msgq) {
		head = head->next;
		found = 1;
	} else {
		for (tmp = head; tmp != NULL && tmp->next != NULL; tmp = tmp->next) {
			if (tmp->next == msgq) {
				tmp->next = tmp->next->next;
				found = 1;
			}
		}
	}
	h2_mutex_unlock(&lock);
	if (found == 0) return EINVALID;
	//h2_pipe_delete(msgq->blastpipe);
	h2_free(msgq->blastpipe);
	h2_free(msgq);
	return EOK;
}

int qmsgq_send_fast(qmsgq_t msgq, int msgtype, const void * __restrict__ data, size_t size, qobject_t obj)
{
	static signed char unique = -1;
	int idx;
	const unsigned long long int * __restrict__ llp = data;
	if (obj) return qmsgq_send_slow (msgq, msgtype, data, size, obj);
	if (((unsigned int)data) & 7) return qmsgq_send_slow(msgq,msgtype,data,size, obj);
	if (size > sizeof(cacheline_t)) return qmsgq_send_slow(msgq,msgtype,data,size, obj);
	_fast_msg_info_t msg;
	idx = shortbuf_getbuf_lockless();
	if (__builtin_expect((idx >= 32),0)) {
		return qmsgq_send_slow(msgq,msgtype,data,size, obj); // oops, full...
	}
	msg.all_fs = 0xff;
	msg.idx = idx;
	msg.len = size;
	msg.id = unique--;		// unsafe, but it's just an ID to help debugging...
	msg.msgtype = msgtype;
	//EJP: I'm unsure: asm (" dczeroa(%0)":"memory":"r"(shortbuf+idx));
    if( size > 0 ) shortbuf[idx].dw0 = llp[0];
	if (size >  8) shortbuf[idx].dw1 = llp[1];
	if (size > 16) shortbuf[idx].dw2 = llp[2];
	if (size > 24) shortbuf[idx].dw3 = llp[3];
	DEBUGMSG("Sending fast msg type=%d id=%d\n",msg.msgtype,msg.id);
	h2_pipe_send(msgq->blastpipe,msg.raw);
	return EOK;
}

int qmsgq_send_slow(qmsgq_t msgq, int msgtype, const void *data, size_t size, qobject_t obj)
{
	static int unique = 0;
	int myid = unique;
	_msg_header_t *realmsg;
	_msg_info_t msg;
	unique++;			// unsafe, but it's just an ID to help debugging...
	if ((realmsg = h2_malloc(size+sizeof(*realmsg))) == NULL) {
		DEBUGMSG("Out of memory!?!?!\n");
		return EMEM;
	}
	realmsg->msgtype = msgtype;
	realmsg->id = myid;
	realmsg->obj = obj;
	memcpy(realmsg+1,data,size);
	msg.msgptr = realmsg;
	msg.len = size;
	DEBUGMSG("Sending msg type=%d id=%d\n",realmsg->msgtype,realmsg->id);
	h2_pipe_send(msgq->blastpipe,msg.raw);
	return EOK;
}

int qmsgq_free_message_slow(const void *msg)
{
	_msg_header_t *realmsg = (void *)msg;
	realmsg--;
	DEBUGMSG("Freeing msg type=%d id=%d\n",realmsg->msgtype,realmsg->id);
	h2_free(realmsg);
	return EOK;
}

int qmsgq_free_message(const void *msg) {
	// we're assuming: fast
	const cacheline_t *p = msg;
	int idx = (p-shortbuf);
	if (idx < 0) return qmsgq_free_message_slow(msg);
	if (idx >= N_SHORTBUFS) return qmsgq_free_message_slow(msg);
	DEBUGMSG("Freeing Fast msg idx=%d\n",idx);
	shortbuf_clearbit_lockless(idx);
	return EOK;
}

int qchannel_create(qch_t *ch)
{
	if ((*ch = h2_malloc(sizeof(*ch))) == NULL) return EMEM;
	return EOK;
}

int qchannel_delete(qch_t ch)
{
	h2_free(ch);
	return EOK;
}

int qchannel_recv_slow(qmsgq_t msgq, int *msgtype, void **data, size_t *msgsize, _msg_info_t msg, qobject_t *obj)
{
	_msg_header_t *msginfo;
	msginfo = msg.msgptr;
	DEBUGMSG("Received msg type=%d id=%d\n",msginfo->msgtype,msginfo->id);
	*msgsize = msg.len;
	*msgtype = msginfo->msgtype;
	if (obj)
		*obj = msginfo->obj;
	*data = msginfo+1;
	return EOK;
}
int qchannel_recv_fast(qmsgq_t msgq, int *msgtype, void **data, size_t *msgsize, unsigned long long int msgraw)
{
	_fast_msg_info_t msg;
	msg.raw = msgraw;
	DEBUGMSG("Received fast msg type=%d id=%d\n",msg.msgtype,msg.id);
	*msgsize = msg.len;
	*msgtype = msg.msgtype;
	*data = shortbuf+msg.idx;
	return EOK;
}

int qchannel_recv(qch_t ch, int *msgtype, void **data, size_t *msgsize, qobject_t *obj)
{
	_msg_info_t msg;
	qmsgq_t msgq = *ch;
	DEBUGMSG("qchan_recv: ch=%x msgq=%x msgq->blastpipe=%x\n",ch,msgq,msgq->blastpipe);
	msg.raw = h2_pipe_recv(msgq->blastpipe);
	if (__builtin_expect(((unsigned int)msg.raw & 7) != 0,1)) {
		int ret = qchannel_recv_fast(msgq,msgtype,data,msgsize,msg.raw);
		if (obj)
			*obj = NULL;
		return ret;
	}
	else return qchannel_recv_slow(msgq,msgtype,data,msgsize,msg,obj);
}

int qchannel_tryrecv(qch_t ch, int *msgtype, void **data, size_t *msgsize, qobject_t *obj)
{
	_msg_info_t msg;
	qmsgq_t msgq = *ch;
	int success;
	msg.raw = h2_pipe_tryrecv(msgq->blastpipe,&success);
	if (!success) return ENO_MSGS;
	if (__builtin_expect(((unsigned int)msg.raw & 7) != 0, 1)) {
		int ret = qchannel_recv_fast(msgq,msgtype,data,msgsize,msg.raw);
		if (obj)
			*obj = NULL;
		return ret;
	}
	else return qchannel_recv_slow(msgq,msgtype,data,msgsize,msg,obj);
}

