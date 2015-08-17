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
#ifndef NETMOD_AMSTUB_RECV_H_INCLUDED
#define NETMOD_AMSTUB_RECV_H_INCLUDED

#include "impl.h"

static inline int MPIDI_netmod_recv(void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPID_Comm * comm,
                                    int context_offset,
                                    MPI_Status * status, MPID_Request ** request)
{
    int err = MPI_SUCCESS;
    MPIU_Assert(0);
    return err;
}

static inline int MPIDI_netmod_recv_init(void *buf,
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


static inline int MPIDI_netmod_mrecv(void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     MPID_Request * message, MPI_Status * status)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}


static inline int MPIDI_netmod_imrecv(void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      MPID_Request * message, MPID_Request ** rreqp)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_irecv(void *buf,
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

static inline int MPIDI_netmod_cancel_recv(MPID_Request * rreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* NETMOD_AMSTUB_RECV_H_INCLUDED */