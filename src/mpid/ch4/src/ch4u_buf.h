/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#ifndef MPIDCH4U_BUF_H_INCLUDED
#define MPIDCH4U_BUF_H_INCLUDED

#include "ch4_impl.h"
#include "ch4u_util.h"
#include <pthread.h>

/* 
   initial prototype of buffer pool. 

   TODO: 
   - align buffer region
   - add garbage collection
   - use huge pages
*/

static inline MPIU_buf_pool_t *MPIU_CH4I_create_buf_pool(int num, int size,
                                                         MPIU_buf_pool_t *parent_pool)
{
    int i;
    MPIU_buf_pool_t *buf_pool;
    MPIU_buf_t *curr, *next;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_CREATE_BUF_POOL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_CREATE_BUF_POOL);

    buf_pool = (MPIU_buf_pool_t *) MPIU_Malloc(sizeof(*buf_pool));
    MPIU_Assert(buf_pool);
    pthread_mutex_init(&buf_pool->lock, NULL);

    buf_pool->size = size;
    buf_pool->num = num;
    buf_pool->next = NULL;
    buf_pool->memory_region = MPIU_Malloc(num * (sizeof(MPIU_buf_t) + size));
    MPIU_Assert(buf_pool->memory_region);

    curr = (MPIU_buf_t *)buf_pool->memory_region;
    buf_pool->head = curr;
    for (i = 0; i < num - 1; i++) {
        next = (MPIU_buf_t *)((char *)curr + size + sizeof(MPIU_buf_t));
        curr->next = next;
        curr->pool = parent_pool ? parent_pool : buf_pool;
        curr = curr->next;
    }
    curr->next = NULL;
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_CREATE_BUF_POOL);
    return buf_pool;
}

static inline MPIU_buf_pool_t *MPIU_CH4U_create_buf_pool(int num, int size)
{
    MPIU_buf_pool_t *buf_pool;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_CREATE_BUF_POOL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_CREATE_BUF_POOL);

    buf_pool = MPIU_CH4I_create_buf_pool(num, size, NULL);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_CREATE_BUF_POOL);
    return buf_pool;
}

static inline void *MPIU_CH4I_get_head_buf(MPIU_buf_pool_t *pool)
{
    void *buf;
    MPIU_buf_t *curr;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_GET_HEAD_BUF);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_GET_HEAD_BUF);

    curr = pool->head;
    pool->head = curr->next;
    buf = curr->data;
    
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_GET_HEAD_BUF);
    return buf;    
}

static inline void *MPIU_CH4U_get_buf_safe(MPIU_buf_pool_t *pool)
{
    void *buf;
    MPIU_buf_pool_t *curr_pool;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_GET_BUF_SAFE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_GET_BUF_SAFE);
    
    if (pool->head) {
        buf = MPIU_CH4I_get_head_buf(pool);
        goto fn_exit;
    }

    curr_pool = pool;
    while (curr_pool->next)
        curr_pool = curr_pool->next;
    
    curr_pool->next = MPIU_CH4I_create_buf_pool(pool->num, pool->size, pool);
    MPIU_Assert(curr_pool->next);
    pool->head = curr_pool->next->head;
    buf = MPIU_CH4I_get_head_buf(pool);

fn_exit:    
    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_GET_BUF_SAFE);
    return buf;
}


static inline void *MPIU_CH4U_get_buf(MPIU_buf_pool_t *pool)
{
    void *buf;

    MPIDI_STATE_DECL(MPID_STATE_CH4U_GET_BUF);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_GET_BUF);

    pthread_mutex_lock(&pool->lock);
    buf = MPIU_CH4U_get_buf_safe(pool);
    pthread_mutex_unlock(&pool->lock);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_GET_BUF);
    return buf;
}

static inline void MPIU_CH4U_release_buf_safe(void *buf)
{
    MPIU_buf_t *curr_buf;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RELEASE_BUF_SAFE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RELEASE_BUF_SAFE);

    curr_buf = container_of(buf, MPIU_buf_t, data);
    curr_buf->next = curr_buf->pool->head;
    curr_buf->pool->head = curr_buf;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RELEASE_BUF_SAFE);
}

static inline void MPIU_CH4U_release_buf(void *buf)
{
    MPIU_buf_t *curr_buf;
    MPIDI_STATE_DECL(MPID_STATE_CH4U_RELEASE_BUF);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4U_RELEASE_BUF);

    curr_buf = container_of(buf, MPIU_buf_t, data);
    pthread_mutex_lock(&curr_buf->pool->lock);
    curr_buf->next = curr_buf->pool->head;
    curr_buf->pool->head = curr_buf;
    pthread_mutex_unlock(&curr_buf->pool->lock);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4U_RELEASE_BUF);
}


static inline void MPIU_CH4U_destroy_buf_pool(MPIU_buf_pool_t *pool)
{
    if (pool->next)
        MPIU_CH4U_destroy_buf_pool(pool->next);
    
    MPIU_Free(pool->memory_region);
    MPIU_Free(pool);
}

#endif /* MPIDCH4_BUF_H_INCLUDED */
