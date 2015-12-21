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
#ifndef CH4_SHM_STUB_REQUEST_H_INCLUDED
#define CH4_SHM_STUB_REQUEST_H_INCLUDED

#include "ch4_shm_impl.h"

static inline int MPIDI_shm_request_is_anysource(MPID_Request * req)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_shm_request_is_pending_failure(MPID_Request * req)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline void MPIDI_shm_request_set_completed(MPID_Request * req)
{
    MPIU_Assert(0);
    return;
}

static inline void MPIDI_shm_request_release(MPID_Request * req)
{
    MPIU_Assert(0);
    return;
}

static inline MPID_Request *MPIDI_shm_request_create(void)
{
    MPID_Request *req = NULL;
    MPIU_Assert(0);
    return req;
}

static inline int MPIDI_shm_anysource_matched(MPID_Request *req,
                                              int *is_cancelled)
{
    MPIU_Assert(0);
    return 0;
}

#endif /* SHM_REQUEST_H_INCLUDED */
