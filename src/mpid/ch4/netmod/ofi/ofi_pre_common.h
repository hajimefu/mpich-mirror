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

#ifndef NETMOD_OFI_PRE_COMMON_H_INCLUDED
#define NETMOD_OFI_PRE_COMMON_H_INCLUDED

#include <mpi.h>
#include "mpihandlemem.h"

#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
#define MPIDI_MAX_ENDPOINTS 256
#define MPIDI_MAX_ENDPOINTS_BITS 8
typedef struct MPIDI_VCR {
    unsigned is_local:1;
    unsigned ep_idx:MPIDI_MAX_ENDPOINTS_BITS;
    unsigned addr_idx:(31 - MPIDI_MAX_ENDPOINTS_BITS);
} MPIDI_VCR;
#else
#define MPIDI_MAX_ENDPOINTS 0
#define MPIDI_MAX_ENDPOINTS_BITS 0
typedef struct MPIDI_VCR {
    unsigned is_local:1;
    unsigned addr_idx:31;
} MPIDI_VCR;
#endif

struct MPIDI_VCRT {
    MPIU_OBJECT_HEADER;
    unsigned size;                /**< Number of entries in the table */
    MPIDI_VCR vcr_table[0];       /**< Array of virtual connection references */
};
typedef struct MPIDI_VCRT *MPID_VCRT;

typedef struct {
    MPID_VCRT  vcrt;
    MPID_VCRT  local_vcrt;
    uint32_t   window_instance;
    void      *huge_send_counters;
    void      *huge_recv_counters;
} MPIDI_netmod_ofi_comm_t;

#include "ofi_pre_common.h"

#endif
