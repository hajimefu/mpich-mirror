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

#ifndef NETMOD_UCX_PRE_COMMON_H_INCLUDED
#define NETMOD_UCX_PRE_COMMON_H_INCLUDED

#include <mpi.h>
#include "mpihandlemem.h"

typedef struct MPIDI_VEP {
    unsigned is_local:1;
    unsigned addr_idx:31;
} MPIDI_VEP;

struct MPIDI_VEPT {
    MPIU_OBJECT_HEADER;
    unsigned size;                /**< Number of entries in the table */
    MPIDI_VEP vep_table[0];       /**< Array of virtual connection references */
};
typedef struct  MPIDI_VEPT * MPIDI_CH4_NMI_UCX_VEP_t;
typedef struct {
    MPIDI_CH4_NMI_UCX_VEP_t vept;
    MPIDI_CH4_NMI_UCX_VEP_t local_vept;
} MPIDI_CH4_NMI_UCX_comm_t;


#endif
