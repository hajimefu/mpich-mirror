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
#ifndef CH4_SHM_STUB_RECV_H_INCLUDED
#define CH4_SHM_STUB_RECV_H_INCLUDED

#include "ch4_shm_impl.h"

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_CH4_SHM_recv)
static inline int MPIDI_CH4_SHM_recv(void *buf,
                                 int count,
                                 MPI_Datatype datatype,
                                 int rank,
                                 int tag,
                                 MPID_Comm * comm,
                                 int context_offset, MPI_Status * status, MPID_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_recv_init(void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_CH4_SHM_mrecv(void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  MPID_Request * message, MPI_Status * status)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_CH4_SHM_imrecv(void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   MPID_Request * message, MPID_Request ** rreqp)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_CH4_SHM_irecv)
static inline int MPIDI_CH4_SHM_irecv(void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  int rank,
                                  int tag,
                                  MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_cancel_recv(MPID_Request * rreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_RECV_H_INCLUDED */
