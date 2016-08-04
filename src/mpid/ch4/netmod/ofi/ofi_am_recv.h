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
#ifndef NETMOD_AM_OFI_RECV_H_INCLUDED
#define NETMOD_AM_OFI_RECV_H_INCLUDED

#include "ofi_impl.h"

static inline int MPIDI_NM_recv(void *buf,
                                    int count,
                                    MPI_Datatype datatype,
                                    int rank,
                                    int tag,
                                    MPIR_Comm *comm,
                                    int context_offset,
                                    MPI_Status *status, MPIR_Request **request)
{
    return MPIDI_CH4U_recv(buf, count, datatype, rank, tag, comm, context_offset, status, request);
}

static inline int MPIDI_NM_recv_init(void *buf,
                                         int count,
                                         MPI_Datatype datatype,
                                         int rank,
                                         int tag,
                                         MPIR_Comm *comm,
                                         int context_offset, MPIR_Request **request)
{
    return MPIDI_CH4U_recv_init(buf, count, datatype, rank, tag, comm, context_offset, request);
}

static inline int MPIDI_NM_imrecv(void *buf,
                                      int count,
                                      MPI_Datatype datatype,
                                      MPIR_Request *message, MPIR_Request **rreqp)
{
    return MPIDI_CH4U_imrecv(buf, count, datatype, message, rreqp);
}

static inline int MPIDI_NM_irecv(void *buf,
                                     int count,
                                     MPI_Datatype datatype,
                                     int rank,
                                     int tag,
                                     MPIR_Comm *comm, int context_offset, MPIR_Request **request)
{
    return MPIDI_CH4U_irecv(buf, count, datatype, rank, tag, comm, context_offset, request);

}

static inline int MPIDI_NM_cancel_recv(MPIR_Request *rreq)
{
    return MPIDI_CH4U_cancel_recv(rreq);
}

#endif /* NETMOD_AM_OFI_RECV_H_INCLUDED */