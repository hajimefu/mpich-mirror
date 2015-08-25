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
#ifndef MPIDCH4_SPAWN_H_INCLUDED
#define MPIDCH4_SPAWN_H_INCLUDED

#include "ch4_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_spawn_multiple
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_spawn_multiple(int count,
                                             char *array_of_commands[],
                                             char **array_of_argv[],
                                             const int array_of_maxprocs[],
                                             MPID_Info * array_of_info_ptrs[],
                                             int root,
                                             MPID_Comm * comm_ptr,
                                             MPID_Comm ** intercomm, int array_of_errcodes[])
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_SPAWN_MULTIPLE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_SPAWN_MULTIPLE);
    MPIU_Assert(0);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_SPAWN_MULTIPLE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_connect
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_connect(const char *port_name,
                                      MPID_Info * info,
                                      int root, MPID_Comm * comm, MPID_Comm ** newcomm_ptr)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_CONNECT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_CONNECT);
    mpi_errno = MPIDI_netmod_comm_connect(port_name, info, root, comm, newcomm_ptr);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_CONNECT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_disconnect
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_disconnect(MPID_Comm * comm_ptr)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_DISCONNECT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_DISCONNECT);
    mpi_errno = MPIDI_netmod_comm_disconnect(comm_ptr);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_DISCONNECT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Open_port
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Open_port(MPID_Info * info_ptr, char *port_name)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_OPEN_PORT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OPEN_PORT);
    mpi_errno = MPIDI_netmod_open_port(info_ptr, port_name);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OPEN_PORT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Close_port
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Close_port(const char *port_name)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_CLOSE_PORT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_CLOSE_PORT);
    mpi_errno = MPIDI_netmod_close_port(port_name);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_CLOSE_PORT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_accept
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_accept(const char *port_name,
                                     MPID_Info * info,
                                     int root, MPID_Comm * comm, MPID_Comm ** newcomm_ptr)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_ACCEPT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_ACCEPT);
    mpi_errno = MPIDI_netmod_comm_accept(port_name, info, root, comm, newcomm_ptr);
    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_ACCEPT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* MPIDCH4_SPAWN_H_INCLUDED */
