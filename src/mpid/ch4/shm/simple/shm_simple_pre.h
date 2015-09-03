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

#ifndef SHM_SIMPLE_PRE_H_INCLUDED
#define SHM_SIMPLE_PRE_H_INCLUDED

#include <mpi.h>

struct MPID_Request;

typedef struct {
    struct MPID_Request *next;
    int dest;
    int rank;
    int tag;
    int context_id;
    char *user_buf;
    int data_sz;
    int type;
    int user_count;
    MPI_Datatype datatype;
} MPIDI_shm_simple_request_t;

typedef struct MPIDI_shm_simple_comm_t {
    struct MPIDI_SHM_VCRT *vcrt;
    struct MPIDI_SHM_VCRT *local_vcrt;
} MPIDI_shm_simple_comm_t;

#endif
