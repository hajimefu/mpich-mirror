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
#ifndef MPIDCH4_PROBE_H_INCLUDED
#define MPIDCH4_PROBE_H_INCLUDED

#include "ch4_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_Probe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Probe(int source,
                               int tag, MPID_Comm * comm, int context_offset, MPI_Status * status)
{
    int mpi_errno, flag = 0;
    MPIDI_STATE_DECL(MPID_STATE_CH4_PROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_PROBE);
    while (!flag) {
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
        mpi_errno = MPIDI_netmod_iprobe(source, tag, comm, context_offset, &flag, status);
#else
        if (MPIDI_CH4U_rank_is_local(source, comm))
            mpi_errno = MPIDI_shm_iprobe(source, tag, comm, context_offset, &flag, status);
        else
            mpi_errno = MPIDI_netmod_iprobe(source, tag, comm, context_offset, &flag, status);
#endif
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
        MPIDI_netmod_progress(MPIDI_CH4_Global.netmod_context[0], 0);
#else
        if (MPIDI_CH4U_rank_is_local(source, comm))
            MPIDI_shm_progress(0);
        else
            MPIDI_netmod_progress(MPIDI_CH4_Global.netmod_context[0], 0);
#endif
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_PROBE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_Mprobe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Mprobe(int source,
                                int tag,
                                MPID_Comm * comm,
                                int context_offset, MPID_Request ** message, MPI_Status * status)
{
    int mpi_errno, flag = 0;
    MPIDI_STATE_DECL(MPID_STATE_CH4_MPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_MPROBE);

    if (source == MPI_PROC_NULL)
    {
        MPIR_Status_set_procnull(status);
        flag = 1;
        *message = NULL; /* should be interpreted as MPI_MESSAGE_NO_PROC */
        goto fn_exit;
    }

    while (!flag) {
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
        mpi_errno = MPIDI_netmod_improbe(source, tag, comm, context_offset, &flag, message, status);
#else
        if (MPIDI_CH4U_rank_is_local(source, comm))
            mpi_errno = MPIDI_shm_improbe(source, tag, comm, context_offset, &flag, message, status);
        else
            mpi_errno = MPIDI_netmod_improbe(source, tag, comm, context_offset, &flag, message, status);
#endif
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
        }
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
        MPIDI_netmod_progress(MPIDI_CH4_Global.netmod_context[0], 0);
#else
        if (MPIDI_CH4U_rank_is_local(source, comm))
            MPIDI_shm_progress(0);
        else
            MPIDI_netmod_progress(MPIDI_CH4_Global.netmod_context[0], 0);
#endif
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_MPROBE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Improbe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Improbe(int source,
                                 int tag,
                                 MPID_Comm * comm,
                                 int context_offset,
                                 int *flag, MPID_Request ** message, MPI_Status * status)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_IMPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_IMPROBE);

    if (source == MPI_PROC_NULL)
    {
        MPIR_Status_set_procnull(status);
        *flag = 1;
        *message = NULL; /* should be interpreted as MPI_MESSAGE_NO_PROC */
        goto fn_exit;
    }

#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_improbe(source, tag, comm, context_offset, flag, message, status);
#else
    if (MPIDI_CH4U_rank_is_local(source, comm))
        mpi_errno = MPIDI_shm_improbe(source, tag, comm, context_offset, flag, message, status);
    else
        mpi_errno = MPIDI_netmod_improbe(source, tag, comm, context_offset, flag, message, status);
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_IMPROBE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Iprobe
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Iprobe(int source,
                                int tag,
                                MPID_Comm * comm,
                                int context_offset, int *flag, MPI_Status * status)
{

    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_IPROBE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_IPROBE);
#ifndef MPIDI_CH4_EXCLUSIVE_SHM
    mpi_errno = MPIDI_netmod_iprobe(source, tag, comm, context_offset, flag, status);
#else
    if (MPIDI_CH4U_rank_is_local(source, comm))
        mpi_errno = MPIDI_shm_iprobe(source, tag, comm, context_offset, flag, status);
    else
        mpi_errno = MPIDI_netmod_iprobe(source, tag, comm, context_offset, flag, status);
#endif
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_IPROBE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* MPIDCH4_WIN_H_INCLUDED */
