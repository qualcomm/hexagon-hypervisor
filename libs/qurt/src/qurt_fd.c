/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt_lifo.h>
#include <qurt_fd.h>
#include <stdint.h>

/* EJP: the best thing about this file is that most of the things are commented out. */

#define DEBUG_ASSERT(...)
//#define DEBUG_ASSERT qube_assert
//
//
void *(memcpy)(void * s1, const void * s2, size_t n);

#define QFD_DEBUG(...)
//#define QFD_DEBUG kprintf

/* FD table/Access list table Implementation 
 *
 * There are two type of fd table: (1) thread/space id table, which doesn't
 * have a access control for each asid. (2) Qube objects talbe which needs
 * finer access control for each asid, which is conceptually a sub-class of (1)
 *
 * Instead of using fancy c++ object virtual functions for general
 * implementaion, we just give two similar code for performance 
 */ 

//
//void qurt_fd_table_init( struct qurt_fd_table *table ){
//
//    //A fd_table starts from a 4K page
//    table->fd = (struct qurt_fd *)heap_alloc(SIZE_4K);
//    table->size = SIZE_4K/sizeof(struct qurt_fd);
//
//    /* Init the structure */
//    int i;
//    for( i = 0 ; i < table->size ; i++ ){
//        qurt_fd_init( &((table)->fd[i] ) );
//        qurt_lifo_push( &(table->free_list), &((table)->fd[i] ) );
//    }
//}

void qurt_fd_table_init_with_mem( struct qurt_fd_table *table, void *mem, unsigned int size ){

    //A fd_table starts from a 4K page
    table->fd = (struct qurt_fd *)mem;
    table->size = size/sizeof(struct qurt_fd);

    /* Init the structure */
    int i;
    for( i = 0 ; i < table->size ; i++ ){
        qurt_fd_init( &((table)->fd[i] ) );
        qurt_lifo_push( &(table->free_list), &((table)->fd[i] ) );
    }
}

void qurt_fd_table_expand_with_mem( struct qurt_fd_table *table, void *mem, unsigned int size, void **old_mem, unsigned int *old_byte_size ){
//    if( table->size == SIZE_64K/sizeof(qurt_fd_t)){
//        //alist table can only be extended once.
//        qube_fatal(E_QFD_TABLE_EMPTY);
//    }

    struct qurt_fd * old_page = table->fd;
    *old_mem = old_page;
    unsigned int old_size = table->size;
    *old_byte_size = old_size * sizeof(struct qurt_fd);

    table->fd = (struct qurt_fd *)mem;
    table->size = size/sizeof(struct qurt_fd);

    /* Copy old page into new fd */
    (memcpy)( (void *)(table->fd), (void *)old_page, old_size * sizeof(struct qurt_fd) );

    /* Init the left over memory */
    int i;
    for( i = old_size ; i < table->size ; i++ ){
        qurt_fd_init( &((table)->fd[i] ) );
        qurt_lifo_push( &(table->free_list), &((table)->fd[i] ) );
    }

    return; 
    // Free the old memory
    //heap_free(old_page);
}

//void qurt_fd_table_expand( struct qurt_fd_table *table ){
//    if( table->size == SIZE_64K/sizeof(qurt_fd_t)){
//        //alist table can only be extended once.
//        qube_fatal(E_QFD_TABLE_EMPTY);
//    }
//
//    struct qurt_fd * old_page = table->fd;
//    //A table starts from a 4K page
//    table->fd = (struct qurt_fd *)heap_big_alloc(SIZE_64K);
//    table->size = SIZE_64K/sizeof(qurt_fd_t);
//
//    /* Copy old page into new fd */
//    (memcpy)( (void *)(table->fd), (void *)old_page, SIZE_4K );
//
//    /* Init the left over memory */
//    int i;
//    for( i = SIZE_4K/sizeof(qurt_fd_t) ; i < table->size ; i++ ){
//        qurt_fd_init( &((table)->fd[i] ) );
//        qurt_lifo_push( &(table->free_list), &((table)->fd[i] ) );
//    }
//
//    // Free the old memory
//    heap_free(old_page);
//}

qurt_obj_t qurt_fd_table_add( struct qurt_fd_table *self, void * handle ){
    qurt_fd_t * new_fd = qurt_lifo_pop( &(self->free_list ) );
    if(!new_fd){
        qurt_obj_t a;
        a.raw = 0;
        return a;
        //qurt_fd_table_expand( self );
        //new_fd = qurt_lifo_pop( &(self->free_list ) );
    }
    //Size of fd has to be 8
    int index = ((uint32_t)new_fd-(uint32_t)self->fd)>>3;
    QFD_DEBUG( "new fd %p base %p index %d\n" , new_fd, self->fd, index );
    qurt_fd_new( new_fd, handle, index);
    return new_fd->obj;
}

void qurt_fd_table_remove( struct qurt_fd_table *self, qurt_obj_t obj ){
    unsigned int index = obj.X.index;
    //Clear myself
    qurt_fd_clear( &(self->fd[index]) );
    qurt_lifo_push( &(self->free_list), &(self->fd[index]) );
}

//void qurt_fd_alist_table_init( struct qurt_fd_alist_table *table ){
//    //A table starts from a 4K page
//    table->fd = (struct qurt_fd_alist *)heap_alloc(SIZE_4K);
//    table->size = SIZE_4K/sizeof(qurt_fd_alist_t);
//
//    /* Init the structure */
//    int i;
//    for( i = 0 ; i < table->size ; i++ ){
//        qurt_fd_alist_init( &((table)->fd[i] ) );
//        qurt_lifo_push( &(table->free_list), &((table)->fd[i] ) );
//    }
//}
//
//void qurt_fd_alist_table_expand( struct qurt_fd_alist_table *table ){
//    if( table->size == SIZE_64K/sizeof(qurt_fd_alist_t)){
//        //alist table can only be extended once.
//        qube_fatal(E_QFD_TABLE_EMPTY);
//    }
//
//    struct qurt_fd_alist * old_page = table->fd;
//    //A table starts from a 4K page
//    table->fd = (struct qurt_fd_alist *)heap_big_alloc(SIZE_64K);
//    table->size = SIZE_64K/sizeof(qurt_fd_alist_t);
//
//    /* Copy old page into new fd */
//    (memcpy)( (void *)(table->fd), (void *)old_page, SIZE_4K );
//
//    /* Init the left over memory */
//    int i;
//    for( i = SIZE_4K/sizeof(qurt_fd_alist_t) ; i < table->size ; i++ ){
//        qurt_fd_alist_init( &((table)->fd[i] ) );
//        qurt_lifo_push( &(table->free_list), &((table)->fd[i] ) );
//    }
//}
//
//qurt_obj_t qurt_fd_alist_table_add( struct qurt_fd_alist_table *self, void * handle, unsigned int magic, int owner_asid){
//    qurt_fd_alist_t * new_fd = qurt_lifo_pop( &(self->free_list ) );
//    if(!new_fd){
//        qurt_fd_alist_table_expand( self );
//        new_fd = qurt_lifo_pop( &(self->free_list ) );
//    }
//    //Size of fd has to be 32 
//    int index = ((u32_t)new_fd-(u32_t)self->fd)>>5;
//    QFD_DEBUG( "new fd %p base %p index %d\n" , new_fd, self->fd, index );
//    qurt_fd_alist_new( new_fd, handle, index, magic, owner_asid);
//    return new_fd->obj;
//}
//
//void qurt_fd_alist_table_remove( struct qurt_fd_alist_table *self, qurt_obj_t obj ){
//    unsigned int index = obj.X.index;
//    //Clear myself
//    qurt_fd_alist_clear( &(self->fd[index]) );
//    qurt_lifo_push( &(self->free_list), &(self->fd[index]) );
//    QFD_DEBUG( "delete fd %p base %p index %d\n" , &(self->fd[index]), self->fd, index );
//}
//

/* fd table is a system resource and we never delete a fd table */
#if  0
int qurt_fd_table_delete( struct qurt_fd_table *fd_table ){
    if(fd_table){
        free(fd_table);
        return EOK;
    }
    else{
        qube_fatal("fd_table is NULL!\n");
    }
}
#endif

//int qurt_fd_table_verify_index( struct qurt_fd_table *self, int pd_index, unsigned int index , unsigned int magic){
//    qobj_index_t obj;
//    obj.raw = index;
//    if( obj.X.index >= self->size){
//        return EINVALID_HANDLE;
//    }
//    if( self->fd[obj.X.index].magic != magic){
//        return EINVALID_HANDLE;
//    }
//    if( obj.X.version_number != self->fd[obj.X.index].obj.X.version_number){
//        return EINVALID_HANDLE;
//    }
//    if( self->fd[obj.X.index].pd_mask_l2[pd_index/32] & (~(1<<(pd_index%32))) == 0 ){
//        return EINVALID_HANDLE;
//    }
//    return EOK;
//}
//
//

//void * qurt_fd_table_get_handle( struct qurt_fd_table *self, unsigned int index , unsigned int magic){
//    qobj_index_t obj;
//    obj.raw = index;
//    return self->fd[obj.X.index].handle;
//}

//int qurt_fd_table_get_magic( struct qurt_fd_table *self, unsigned int index, unsigned int *magic){
//    qobj_index_t obj;
//    obj.raw = index;
//
//    if( obj.X.index >= self->size ){
//        return EINVALID_HANDLE;
//    }
//
//    if( obj.X.version_number != self->fd[obj.X.index].obj.X.version_number){
//        return EINVALID_HANDLE;
//    }
//
//    *magic = self->fd[obj.X.index].magic; 
//    return EOK;
//}

//int qurt_fd_table_remove( struct qurt_fd_table *self, int pd_index, unsigned int in_obj ){
//
//    qobj_index_t obj;
//    obj.raw = in_obj;
//
//    unsigned int index = obj.X.index;
//
//    self->fd[index].pd_mask_l2[pd_index/32] &= ~(1<<(pd_index%32));
//    if(self->fd[index].pd_mask_l2[pd_index/32] == 0){
//        self->fd[index].pd_mask_l1 &= ~(1<<(pd_index/32));
//    }
//
//    if(self->fd[index].pd_mask_l1 == 0){
//        //Clear myself
//        qurt_fd_clear( &(self->fd[index]) );
//    }
//
//    return EOK;
//
//}

//
//int qurt_fd_table_copy( struct qurt_fd_table *t, unsigned int *out_index, int dest_pd_index, int src_pd_index, unsigned int src_index, unsigned int permission ){
//    qobj_index_t obj;
//    obj.raw = src_index;
//    qurt_fd_copy( &(t->fd[obj.X.index]), dest_pd_index, src_pd_index );
//    *out_index = src_index;
//}
//
//int qurt_fd_copy( struct qurt_fd *self, int dest_pd_index, int src_pd_index ){
//    QFD_DEBUG( "copy fd dest %p src %p\n", dest_pd_index, src_pd_index  );
//    self->pd_mask_l2[src_pd_index/32] |= (1<<(src_pd_index%32)); 
//}

//int qurt_fd_alist_clear( struct qurt_fd *self ){
//    QFD_DEBUG( "clear fd %p\n" , self );
//    self->handle = NULL;
//    self->magic = 0;
//    self->pd_mask_l1=0;
//    int i;
//    for( i = 0; i < MAX_PDS/32; i++ ){
//        self->pd_mask_l2[i] = 0;
//    }
//
//    //Only clear index and local fields, never touch version_number
//    self->obj.X.index = 0;
//    self->obj.X.local = QOBJ_LOCAL;
//}
//
//int qurt_fd_table_double( struct qurt_fd_table *fd_table ){
//
//    int new_size = fd_table->size * 2;
//    int old_size = fd_table->size;
//    
//    qurt_printf( "Doubling fd_table from %d to %d\n", old_size, new_size );
//    struct qurt_fd *old_fd = (fd_table)->fd;
//
//    (fd_table)->fd = (struct qurt_fd *)malloc( new_size * sizeof( struct qurt_fd ));   
//    if( !((fd_table)->fd)) {
//        qube_fatal("Unable to allocate memory for enlarged fd table!\n");
//    }
//
//    //Change the size to new size
//    (fd_table)->size = new_size;
//
//    //Keep the old next_index
//    //(fd_table)->next_index = 0;
//
//    /* Copy the old fd_table into new one */
//    (memcpy)( fd_table->fd, old_fd, old_size * sizeof( struct qurt_fd ));
//
//
//    /* Init left of the structure */
//    int i;
//    for( i = old_size ; i < new_size ; i++ ){
//        qurt_fd_init_descriptor( &((fd_table)->fd[i] ) );
//    }
//
//    free( old_fd );
//
//    return EOK;
//}
