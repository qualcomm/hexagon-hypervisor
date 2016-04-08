/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_FD_H
#define QURT_FD_H

/* EJP: why is this here? needed by posix implementation.  Probably needs to be
 * removed if we rewrite posix. */

#include <stdlib.h>

#define FD_INDEX_BITS       (13) //At most 2^13= 8k objects in one fd table
#define FD_VER_NUM_BITS     (8) 
#define FD_REFERENCE_COUNTER_BITS       (10)  //concurrent read
#define FD_TABLE_MAX_SIZE   (2^FD_INDEX_BITS)

typedef enum {
        QOBJ_LOCAL=0,
            QOBJ_GLOBAL
}qurt_obj_type_t; 

typedef union {
    unsigned int raw;
    /* QDSP6 bit field starts from LSB */
    struct {
        unsigned int index:FD_INDEX_BITS;
        unsigned int version_number:(32-1-FD_INDEX_BITS-FD_REFERENCE_COUNTER_BITS); /* This is needed to
                                                             avoid index wrap
                                                             around problem.
                                                             we can reserve
                                                             32-Log(length_of_fd_table)
                                                             bits in handle as
                                                             version number so
                                                             that it only
                                                             happens after
                                                             index wrap around
                                                             2^(32-Log(length_of_fd_table))
                                                             times.*/
        unsigned int local:1;
        int ref_counter:FD_REFERENCE_COUNTER_BITS;
    } X;
}qurt_obj_t;

struct qurt_fd {
    void *              handle;
    qurt_obj_t              obj;
};

typedef struct qurt_fd qurt_fd_t;

struct qurt_fd_table {
    struct qurt_fd *fd;
    struct qurt_fd *free_list;
    unsigned int size;
};

typedef struct qurt_fd_table qurt_fd_table_t;

//typedef struct general_qube_handle {
//    unsigned int dummy;
//    unsigned int magic;        /* The second word of all qube data structure
//                                  will be the magic word, holding type
//                                  information */
//} qhandle_t;
//struct qurt_fd_alist {
//    qhandle_t *         handle;
//    qurt_obj_t              obj;
//    u64_t               pd_mask;
////    unsigned int        magic;          /* This is needed to verify the type of
////                                           the handle, so that server can
////                                           easily figure out the error that
////                                           user give qmemory handle to qmsgq */
//    u32_t               owner_asid;      //Owner space
//    u32_t               pad[3];         //Make it 8 words/32 bytes for quick shifting.  TODO: We should put magic into obj
//};
//
//typedef struct qurt_fd_alist qurt_fd_alist_t;
//
//struct qurt_fd_alist_table {
//    struct qurt_fd_alist *fd;
//    struct qurt_fd_alist *free_list;
//    u32_t size;
//};
//
//typedef struct qurt_fd_alist_table qurt_fd_alist_table_t;

/*
#define qurt_fd_table_is_valid_obj( self, m_obj ) \
    ( (self)->size > m_obj.X.index && (self)->fd[m_obj.X.index].obj.X.version_number == m_obj.X.version_number )
*/
//Faster version of above
#define qurt_fd_table_is_valid_obj( self, m_obj ) \
    ( (self)->size > ((qurt_obj_t)m_obj).X.index && (self)->fd[((qurt_obj_t)m_obj).X.index].obj.raw== ((qurt_obj_t)m_obj).raw)

#define qurt_fd_table_get_handle(self, m_obj ) \
    ((self)->fd[m_obj.X.index].handle)

#define qurt_fd_init( self ) \
do { \
    QFD_DEBUG( "init fd %p\n" , (self) ); \
    (self)->handle = NULL; \
    (self)->obj.raw = 0; \
} while(0)

/*Version number will never be cleared*/ 
#define qurt_fd_new(self, m_handle, m_index) \
do { \
 \
    (self)->handle = m_handle; \
 \
    (self)->obj.X.index = m_index; \
    (self)->obj.X.local = QOBJ_GLOBAL; \
    (self)->obj.X.ref_counter = 0; \
    QFD_DEBUG( "new fd %p handle %p index %p\n", (self), m_handle, m_index ); \
} while(0)

/* Only clear index and local fields */ 
/* Clear/remove an fd entry means version number is not matching anymore */
#define qurt_fd_clear( self ) \
do { \
    QFD_DEBUG( "clear fd %p\n" , (self) ); \
    (self)->handle = NULL; \
    (self)->obj.X.index = 0; \
    (self)->obj.X.version_number ++; \
    (self)->obj.X.local = QOBJ_LOCAL; \
    (self)->obj.X.ref_counter = 0; \
} while(0)

#define qurt_fd_alist_init( self ) \
do { \
    QFD_DEBUG( "init fd_alist %p\n" , (self) ); \
    (self)->handle = NULL; \
    (self)->obj.raw = 0; \
    (self)->pd_mask=0; \
    (self)->owner_asid=0; \
} while(0) 

#define qurt_fd_alist_new(self, m_handle, m_index, m_magic, m_asid ) \
do { \
 \
    (self)->handle = m_handle; \
    (self)->handle->magic = m_magic; \
 \
    (self)->obj.X.ref_counter = 0; \
    (self)->obj.X.index = m_index; \
    (self)->obj.X.local = QOBJ_GLOBAL; \
    (self)->pd_mask = 1<<(m_asid); \
    (self)->owner_asid = (m_asid); \
    QFD_DEBUG( "new fd %p handle %p magic %p owner %p index %p\n", (self), m_handle, m_magic, m_asid, m_index ); \
} while(0)

#define qurt_fd_alist_table_is_valid_obj( self, m_obj, m_magic ) \
    ( (self)->size > m_obj.X.index && (self)->fd[m_obj.X.index].obj.raw == m_obj.raw &&  (self)->fd[m_obj.X.index].handle->magic == (m_magic))

#define qurt_fd_alist_table_get_handle(self, m_obj ) ((self)->fd[m_obj.X.index].handle)
#define qurt_fd_alist_table_get_pd_mask(self, m_obj ) ((self)->fd[m_obj.X.index].pd_mask)
#define qurt_fd_alist_table_is_shared(self, m_obj ) ((self)->fd[m_obj.X.index].pd_mask & (~(1<<(self)->fd[m_obj.X.index].owner_asid)) )

#define qurt_fd_alist_table_grant( self, m_obj, m_asid ) \
    ( (self)->fd[m_obj.X.index].pd_mask |= (1<<m_asid))

#define qurt_fd_alist_table_has_granted( self, m_obj, m_asid ) \
    ( (self)->fd[m_obj.X.index].pd_mask & (1<<m_asid))

#define qurt_fd_alist_table_revoke( self, m_obj, m_asid ) \
    ( (self)->fd[m_obj.X.index].pd_mask & (~(1<<m_asid)))

/*Only clear index and local fields*/ 
/* Clear/remove an fd entry means version number is not matching anymore */
#define qurt_fd_alist_clear( self ) \
do { \
    QFD_DEBUG( "clear fd %p\n" , (self) ); \
    (self)->handle->magic = 0; \
    (self)->handle = NULL; \
    (self)->obj.X.index = 0; \
    (self)->obj.X.local = QOBJ_LOCAL; \
    (self)->obj.X.ref_counter = 0; \
    (self)->pd_mask=0; \
    (self)->owner_asid=0; \
    (self)->obj.X.version_number ++; \
} while(0)

//void qurt_fd_alist_table_init( struct qurt_fd_alist_table *table );
//qurt_obj_t qurt_fd_alist_table_add( struct qurt_fd_alist_table *self, void * handle, unsigned int magic, int owner_asid);
//
//void qurt_fd_table_init( struct qurt_fd_table *table );

//
//
//Qurt FD(file descriptor) table APIs
//
//The first argument will always be the table pointer.
//
//
//This macro return non-zero is the object is a valid one
//#define qurt_fd_table_is_valid_obj( self, m_obj ) 
//
//
//
//This macro return handle or the pointer to your real data structure 
//#define qurt_fd_table_get_handle(self, m_obj ) 
//
//This function will init the table with a pre-allocated memory, with BYTE size
void qurt_fd_table_init_with_mem( struct qurt_fd_table *table, void *mem, unsigned int size );

//This function will will expend the table to a bigger one with new memory pointer and new size
//Old memory and size are saved so that user can free it.
void qurt_fd_table_expand_with_mem( struct qurt_fd_table *table, void *mem, unsigned int new_size, void **old_mem, unsigned int *old_size );

//Add a new pointer to the table, and returns an object
qurt_obj_t qurt_fd_table_add( struct qurt_fd_table *self, void * handle );
//
//Remove the object from the table
void qurt_fd_table_remove( struct qurt_fd_table *self, qurt_obj_t obj );

//The following is the example how we wrap thread_id using fd table
/*
#include <qfd.h>

typedef qfd_table_t QURTK_thread_id_table_t;

typedef qurt_obj_t QURTK_thread_id_t;

extern QURTK_thread_id_table_t QURTK_thread_id_table;
extern void * QURTK_thread_contexts_lifo;

#define QURTK_thread_id_table_init qfd_table_init_with_mem

#define QURTK_thread_id_new( handle ) \
    qfd_table_add( &(QURTK_thread_id_table), (handle))

#define QURTK_thread_id_delete( obj) \
    qfd_table_remove( &(QURTK_thread_id_table), (qurt_obj_t)(obj))

#define QURTK_thread_id_is_valid(obj) \
    qfd_table_is_valid_obj( &(QURTK_thread_id_table),(obj))

#define QURTK_thread_id_get_handle( m_obj ) \
    qfd_table_get_handle( &(QURTK_thread_id_table), (m_obj))

*/

#endif  /* QURT_FD_H */
