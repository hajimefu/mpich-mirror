/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef CH4_SHM_STUB_COMM_H_INCLUDED
#define CH4_SHM_STUB_COMM_H_INCLUDED

#include "ch4_shm_impl.h"
static inline int MPIDI_CH4_SHM_comm_create(MPIR_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIU_Assert(0);
    return mpi_errno;
}
static inline int MPIDI_CH4_SHM_comm_destroy(MPIR_Comm * comm)
{
    int mpi_errno = MPI_SUCCESS;
    MPIU_Assert(0);
    return mpi_errno;
}


#endif /* SHM_COMM_H_INCLUDED */
