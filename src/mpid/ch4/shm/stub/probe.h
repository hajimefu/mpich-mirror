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
#ifndef CH4_SHM_STUB_PROBE_H_INCLUDED
#define CH4_SHM_STUB_PROBE_H_INCLUDED

#include "shm_impl.h"


static inline int MPIDI_CH4_SHM_improbe(int source,
                                    int tag,
                                    MPIR_Comm * comm,
                                    int context_offset,
                                    int *flag, MPIR_Request ** message, MPI_Status * status)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_iprobe(int source,
                                   int tag,
                                   MPIR_Comm * comm,
                                   int context_offset, int *flag, MPI_Status * status)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* SHM_PROBE_H_INCLUDED */
