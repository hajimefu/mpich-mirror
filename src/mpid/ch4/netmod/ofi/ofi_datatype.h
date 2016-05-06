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
#ifndef NETMOD_OFI_DATATYPE_H_INCLUDED
#define NETMOD_OFI_DATATYPE_H_INCLUDED

/*
  These are the stub functions for datatype hooks.
  When actually implementing these make sure to #define
  HAVE_MPIDI_NM_datatype_commit_hook and/or HAVE_MPIDI_NM_datatype_destroy_hook
  in ofi_pre.h
*/

static inline void MPIDI_NM_datatype_destroy_hook(MPIR_Datatype *datatype_p)
{
    return;
}

static inline void MPIDI_NM_datatype_commit_hook(MPI_Datatype *datatype_p)
{
    return;
}

static inline void MPIDI_NM_datatype_dup_hook(MPIR_Datatype *datatype_p)
{
    return;
}

#endif
