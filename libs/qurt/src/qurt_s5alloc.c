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
	do {
		cur = (struct s5_element *)stream->headaddr;
		if (cur == NULL) {
			qurt_s5_morepages(stream,id);
			if (stream->headaddr == 0) return NULL;
			continue;
		}
	} while (cas(&stream->headaddr,cur,cur->next) != (atomic_u32_t)cur);
	return cur;
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
	while (memsize > elsize) {
		qurt_s5_free(id,memptr);
		memptr += elsize;
		memsize -= elsize;
	};
	return 0;
}

