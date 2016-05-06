/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef SHM_STUBSHM_SEND_H_INCLUDED
#define SHM_STUBSHM_SEND_H_INCLUDED

#include "stubshm_impl.h"

static inline int MPIDI_SHM_send(const void *buf,
                                 int count,
                                 MPI_Datatype datatype,
                                 int rank,
                                 int tag,
                                 MPIR_Comm * comm, int context_offset, MPIR_Request ** request)
{
    int err = MPI_SUCCESS;
    MPIU_Assert(0);
    return err;
}




static inline int MPIDI_SHM_irsend(const void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   int rank,
                                   int tag,
                                   MPIR_Comm * comm, int context_offset, MPIR_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_SHM_ssend(const void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  int rank,
                                  int tag,
                                  MPIR_Comm * comm, int context_offset, MPIR_Request ** request)
{
    int err = MPI_SUCCESS;
    MPIU_Assert(0);

    return err;
}

static inline int MPIDI_SHM_startall(int count, MPIR_Request * requests[])
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_SHM_send_init(const void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      int rank,
                                      int tag,
                                      MPIR_Comm * comm, int context_offset, MPIR_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_SHM_ssend_init(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPIR_Comm * comm,
                                       int context_offset, MPIR_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_SHM_bsend_init(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPIR_Comm * comm,
                                       int context_offset, MPIR_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_SHM_rsend_init(const void *buf,
                                       int count,
                                       MPI_Datatype datatype,
                                       int rank,
                                       int tag,
                                       MPIR_Comm * comm,
                                       int context_offset, MPIR_Request ** request)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_SHM_isend)
static inline int MPIDI_SHM_isend(const void *buf,
                                  int count,
                                  MPI_Datatype datatype,
                                  int rank,
                                  int tag,
                                  MPIR_Comm * comm, int context_offset, MPIR_Request ** request)
{
    int err = MPI_SUCCESS;
    MPIU_Assert(0);
    return err;
}

static inline int MPIDI_SHM_issend(const void *buf,
                                   int count,
                                   MPI_Datatype datatype,
                                   int rank,
                                   int tag,
                                   MPIR_Comm * comm, int context_offset, MPIR_Request ** request)
{
    int err = MPI_SUCCESS;
    MPIU_Assert(0);
    return err;
}

static inline int MPIDI_SHM_cancel_send(MPIR_Request * sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_STUBSHM_SEND_H_INCLUDED */
