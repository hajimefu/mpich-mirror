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

#include <mpidimpl.h>
#include "ch4_shm_impl.h"

MPIDI_CH4_SHMI_SIMPLE_Request_queue_t  MPIDI_CH4_SHMI_SIMPLE_Sendq = { NULL, NULL };
MPIDI_CH4_SHMI_SIMPLE_Request_queue_t  MPIDI_CH4_SHMI_SIMPLE_Recvq_posted = { NULL, NULL };
MPIDI_CH4_SHMI_SIMPLE_Request_queue_t  MPIDI_CH4_SHMI_SIMPLE_Recvq_unexpected = { NULL, NULL };
MPIDI_CH4_SHMI_SIMPLE_Mem_region_t     MPIDI_CH4_SHMI_SIMPLE_mem_region = {{0}};
char                                  *MPIDI_CH4_SHMI_SIMPLE_Asym_base_addr = 0;
MPID_Thread_mutex_t                    MPID_shm_mutex;
