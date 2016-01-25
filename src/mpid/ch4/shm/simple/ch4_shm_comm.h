/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef SHM_COMM_H_INCLUDED
#define SHM_COMM_H_INCLUDED

#include "ch4_shm_impl.h"
#include "impl.h"
#include "mpl_utlist.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_comm_create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_comm_create(MPID_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_SHM_COMM_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_SHM_COMM_CREATE);

    MPIDI_FUNC_EXIT(MPID_STATE_SHM_COMM_CREATE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_comm_destroy
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_comm_destroy(MPID_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_SHM_COMM_DESTROY);
    MPIDI_FUNC_ENTER(MPID_STATE_SHM_COMM_DESTROY);

    MPIDI_FUNC_EXIT(MPID_STATE_SHM_COMM_DESTROY);
    return mpi_errno;
}


#endif /* SHM_COMM_H_INCLUDED */
