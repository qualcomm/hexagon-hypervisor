/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt_s5alloc.h>
#include <h2.h>
/*
 * Simple Stream of Same Sized Structs
 *
 * id=s5_create(elsize,more_mem_ptr,opaque): Create a new stream
 * s5_feed(id,mem,memsize): Feed more memory 
 * ptr=s5_alloc(id): allocate a new struct
 * s5_free(id,ptr): free a struct
 *
 * Maximum packing
 * Simple allocation / freeing of elements
 * Can't shrink pools without a lot of work
 */

#define MAX_STREAMS 8

#define cas(x,y,z) h2_atomic_compare_swap32(x,(atomic_u32_t)y,(atomic_u32_t)z)

#ifdef DEBUG_BIST
#define pthread_plainmutex_lock_np(...) /* NOTHING */
#define pthread_plainmutex_unlock_np(...) /* NOTHING */
#endif

struct s5_stream_info {
	union {
		struct {
			atomic_u32_t headaddr;
			unsigned int elementsize;
		};
		unsigned long long int elementsize_head_raw;
	};
	void (*more_mem_ptr)(void *opaque,qurt_s5id_t s5id, unsigned int elementsize);
	void *opaque;
};

struct s5_element {
	struct s5_element *next;
};

static struct s5_stream_info streams[MAX_STREAMS];
pthread_plainmutex_t plainmutex = PTHREAD_PLAINMUTEX_INITIALIZER_NP;

qurt_s5id_t qurt_s5_create(unsigned int elementsize, void (*more_mem_ptr)(void *opaque, qurt_s5id_t s5id, unsigned int elementsize), void *opaque)
{
	struct s5_stream_info *stream = NULL;
	int i;
	/* LOCK */
	pthread_plainmutex_lock_np(&plainmutex);
	for (i = 0; i < MAX_STREAMS; i++) {
		if (streams[i].elementsize == 0) {
			stream = &streams[i];
			stream->elementsize = elementsize;
			break;
		}
	}
	pthread_plainmutex_unlock_np(&plainmutex);
	if (stream == NULL) return -1;
	stream->more_mem_ptr = more_mem_ptr;
	stream->opaque = opaque;
	return i;
}

static inline void qurt_s5_morepages(struct s5_stream_info *stream, qurt_s5id_t id)
{
	if (stream->more_mem_ptr) stream->more_mem_ptr(stream->opaque,id,stream->elementsize);
}

void *qurt_s5_alloc(qurt_s5id_t id)
{
	struct s5_stream_info *stream;
	struct s5_element *cur;
	stream = &streams[id];
	while (1) {
		/* If no memory, ask for more memory */
		if ((cur = (struct s5_element *)stream->headaddr) == NULL) {
			qurt_s5_morepages(stream,id);
			/* If still no memory, out of memory */
			if ((cur = (struct s5_element *)stream->headaddr) == NULL) return NULL;
		}
		/* If we can cas pop the head, we're done.  Otherwise, try again */
		if (cas(&stream->headaddr,cur,cur->next) == (atomic_u32_t)cur) return cur;
	}
}

int qurt_s5_free(qurt_s5id_t id, void *mem)
{
	struct s5_element *cur = mem;
	struct s5_element *next;
	struct s5_stream_info *stream;
	stream = &streams[id];
	do {
		cur->next = next = (struct s5_element *)stream->headaddr;
	} while (cas(&stream->headaddr,next,cur) != (atomic_u32_t)next);
	return 0;
}

int qurt_s5_feed(qurt_s5id_t id, void *mem, unsigned int memsize)
{
	struct s5_stream_info *stream;
	char *memptr;
	unsigned int elsize;
	stream = &streams[id];
	elsize = stream->elementsize;
	memptr = mem;
	while (memsize >= elsize) {
		qurt_s5_free(id,memptr);
		memptr += elsize;
		memsize -= elsize;
	};
	return 0;
}

#ifdef DEBUG_BIST

#include <stdio.h>

static void FAIL(const char *msg)
{
	printf("oops: %s\n",msg);
	exit(1);
}

static unsigned int s5_count_elements(qurt_s5id_t id)
{
	unsigned int count = 0;
	struct s5_stream_info *stream;
	struct s5_element *ptr;
	stream = &streams[id];
	for (ptr = (struct s5_element *)stream->headaddr; ptr != NULL; ptr = ptr->next) count++;
	return count;
}

static void s5_dump()
{
	int i;
	for (i = 0; i < MAX_STREAMS; i++) {
		printf("stream %d: elementsize=%4d head=%08x items=%d\n",
			i,streams[i].elementsize,streams[i].headaddr,s5_count_elements(i));
	}
}

char data[1024];
qurt_s5id_t a,b,c;

int TH_seen_nomore_data = 0;
void TH_nomore_data(void *opaque, qurt_s5id_t id, unsigned int elementsize)
{
	puts("nomore!");
	if ((long)opaque != (long)0xdeadbeef) FAIL("nomore: opaque");
	if (id != b) FAIL("nomore: id");
	if (elementsize != 16) FAIL("nomore: elementsize");
	TH_seen_nomore_data = 1;
}

int TH_seen_more_data = 0;
void TH_more_data(void *opaque, qurt_s5id_t id, unsigned int elementsize)
{
	static int once = 1;
	puts("more!");
	if ((long)opaque != (long)0xcafebabe) FAIL("nomore: opaque");
	if (id != c) FAIL("nomore: id");
	if (elementsize != 32) FAIL("nomore: elementsize");
	if (once) qurt_s5_feed(id,data+128,32);
	once = 0;
}

int main()
{
	s5_dump();
	if ((a = qurt_s5_create(8,NULL,NULL)) < 0) FAIL("create/a");
	if ((b = qurt_s5_create(16,TH_nomore_data,(void *)0xdeadbeef)) < 0) FAIL("create/b");
	if ((c = qurt_s5_create(32,TH_more_data,(void *)0xcafebabe)) < 0) FAIL("create/c");
	printf("streams alloc'd\n");
	s5_dump();
	if (qurt_s5_alloc(a)) FAIL("empty/a");
	if (qurt_s5_alloc(b)) FAIL("empty/b");
	qurt_s5_feed(a,data,8);
	qurt_s5_feed(b,data+16,16);
	qurt_s5_feed(c,data+32,32);
	printf("streams fed\n");
	s5_dump();
	if (qurt_s5_alloc(a) != (void *)(data+0)) FAIL("alloc1/a");
	if (qurt_s5_alloc(b) != (void *)(data+16)) FAIL("alloc1/b");
	if (qurt_s5_alloc(c) != (void *)(data+32)) FAIL("alloc1/c");
	printf("streams empty\n");
	s5_dump();
	if (qurt_s5_alloc(a)) FAIL("empty2/a");
	if (qurt_s5_alloc(b)) FAIL("empty2/b");
	if (!TH_seen_nomore_data) FAIL("empty2/b/nomore call");
	if (qurt_s5_alloc(c) != (void *)(data+128)) FAIL("more data");
	if (qurt_s5_alloc(c)) FAIL("empty2/c");
	puts("PASS");
}
#endif
