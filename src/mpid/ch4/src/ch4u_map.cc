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

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <map>
#include <queue>
#include <new>
#include <mpidimpl.h>
EXTERN_C_BEGIN

typedef std::map<uint64_t,void *>  uint64_map;
void MPIDI_CH4I_map_create(void **_map)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_UTIL_MUTEX);
    *_map = (void *)MPIU_Malloc(sizeof(uint64_map));
    new(*_map) uint64_map();
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_UTIL_MUTEX);
}

void MPIDI_CH4I_map_destroy(void *_map)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_UTIL_MUTEX);
    uint64_map *m = (uint64_map *)_map;
    m->~uint64_map();
    MPIU_Free(_map);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_UTIL_MUTEX);
}

void MPIDI_CH4I_map_set(void     *_map,
                       uint64_t  id,
                       void     *val)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_UTIL_MUTEX);
    uint64_map *m = (uint64_map *)_map;
    (*m)[id] = val;
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_UTIL_MUTEX);
}

void MPIDI_CH4I_map_erase(void     *_map,
                         uint64_t  id)
{
    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_UTIL_MUTEX);
    uint64_map *m = (uint64_map *)_map;
    m->erase(id);
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_UTIL_MUTEX);
}

void *MPIDI_CH4I_map_lookup(void     *_map,
                           uint64_t  id)
{
    uint64_map *m = (uint64_map *)_map;
    void       *rc;

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_THREAD_UTIL_MUTEX);
    if(m->find(id) == m->end())
        rc = MPIDI_CH4I_MAP_NOT_FOUND;
    else
        rc = (*m)[id];
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_THREAD_UTIL_MUTEX);
    return rc;
}

EXTERN_C_END
