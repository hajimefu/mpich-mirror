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
#ifndef NETMOD_OFI_COMM_H_INCLUDED
#define NETMOD_OFI_COMM_H_INCLUDED

#include "ofi_impl.h"
#include "mpl_utlist.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_comm_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_comm_create(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_OFI_COMM_CREATE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_OFI_COMM_CREATE);

    MPIDI_OFI_map_create(&MPIDI_OFI_COMM(comm).huge_send_counters);
    MPIDI_OFI_map_create(&MPIDI_OFI_COMM(comm).huge_recv_counters);
    MPIDI_OFI_index_allocator_create(&MPIDI_OFI_COMM(comm).win_id_allocator,0);
    MPIDI_OFI_index_allocator_create(&MPIDI_OFI_COMM(comm).rma_id_allocator,1);

    mpi_errno = MPIDI_CH4U_init_comm(comm);

    /* Do not handle intercomms */
    if(comm->comm_kind == MPIR_COMM_KIND__INTERCOMM)
        goto fn_exit;

    MPIR_Assert(comm->coll_fns != NULL);
fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_OFI_COMM_CREATE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_NM_comm_destroy
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_NM_comm_destroy(MPIR_Comm *comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_NETMOD_OFI_COMM_DESTROY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_NETMOD_OFI_COMM_DESTROY);

    mpi_errno = MPIDI_CH4U_destroy_comm(comm);
    MPIDI_OFI_map_destroy(MPIDI_OFI_COMM(comm).huge_send_counters);
    MPIDI_OFI_map_destroy(MPIDI_OFI_COMM(comm).huge_recv_counters);
    MPIDI_OFI_index_allocator_destroy(MPIDI_OFI_COMM(comm).win_id_allocator);
    MPIDI_OFI_index_allocator_destroy(MPIDI_OFI_COMM(comm).rma_id_allocator);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_NETMOD_OFI_COMM_DESTROY);
    return mpi_errno;
}


#endif /* NETMOD_OFI_COMM_H_INCLUDED */
