/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef UCX_PRE_COMMON_H_INCLUDED
#define UCX_PRE_COMMON_H_INCLUDED

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
typedef struct  MPIDI_VEPT * MPIDI_UCX_VEP_t;
typedef struct {
    MPIDI_UCX_VEP_t vept;
    MPIDI_UCX_VEP_t local_vept;
} MPIDI_UCX_comm_t;

#endif /* UCX_PRE_COMMON_H_INCLUDED */
