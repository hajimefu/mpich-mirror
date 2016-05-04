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
#ifndef PROC_H_INCLUDED
#define PROC_H_INCLUDED

#include "shm_impl.h"

static inline int MPIDI_CH4_SHM_rank_is_local(int rank, MPIR_Comm * comm)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}
#endif /* PROC_H_INCLUDED */
