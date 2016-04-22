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
#ifndef SHM_DYNPROC_H_INCLUDED
#define SHM_DYNPROC_H_INCLUDED

#include "ch4_shm_impl.h"

static inline int MPIDI_CH4_SHM_comm_connect(const char *port_name,
                                             MPIR_Info *info,
                                             int root, MPIR_Comm *comm, MPIR_Comm **newcomm_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_comm_disconnect(MPIR_Comm *comm_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_open_port(MPIR_Info *info_ptr, char *port_name)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_close_port(const char *port_name)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_comm_accept(const char *port_name,
                                            MPIR_Info *info,
                                            int root, MPIR_Comm *comm, MPIR_Comm **newcomm_ptr)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_WIN_H_INCLUDED */
