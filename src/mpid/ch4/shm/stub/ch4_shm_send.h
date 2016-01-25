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
#ifndef CH4_SHM_STUB_SEND_H_INCLUDED
#define CH4_SHM_STUB_SEND_H_INCLUDED

#include "ch4_shm_impl.h"

static inline int MPIDI_CH4_SHM_send(const void *buf,
                                 int count,
                                 MPI_Datatype datatype,
                                 int rank,
                                 int tag,
                                 MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int err = MPI_SUCCESS;
    MPIU_Assert(0);
    return err;
}

static inline int MPIDI_CH4_SHM_rsend(const void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  int rank,
                                  int tag,
                                  MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int err = MPI_SUCCESS;
    MPIU_Assert(0);
    return err;
}



static inline int MPIDI_CH4_SHM_irsend(const void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   int rank,
                                   int tag,
                                   MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_ssend(const void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  int rank,
                                  int tag,
                                  MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int err = MPI_SUCCESS;
    MPIU_Assert(0);

    return err;
}

static inline int MPIDI_CH4_SHM_startall(int count, MPID_Request * requests[])
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_send_init(const void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_ssend_init(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPID_Comm * comm,
                                       int context_offset, MPID_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_bsend_init(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPID_Comm * comm,
                                       int context_offset, MPID_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_rsend_init(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPID_Comm * comm,
                                       int context_offset, MPID_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_CH4_SHM_isend)
static inline int MPIDI_CH4_SHM_isend(const void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  int rank,
                                  int tag,
                                  MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int err = MPI_SUCCESS;
    MPIU_Assert(0);
    return err;
}

static inline int MPIDI_CH4_SHM_issend(const void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   int rank,
                                   int tag,
                                   MPID_Comm * comm, int context_offset, MPID_Request ** request)
{
    int err = MPI_SUCCESS;
    MPIU_Assert(0);
    return err;
}

static inline int MPIDI_CH4_SHM_cancel_send(MPID_Request * sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_SEND_H_INCLUDED */
