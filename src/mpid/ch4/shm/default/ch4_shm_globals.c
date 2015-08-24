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

MPIDI_shm_queue_t MPIDI_shm_sendq = { NULL, NULL };
MPID_nem_mem_region_t MPID_nem_mem_region = { {0}
};

char *MPID_nem_asymm_base_addr = 0;
MPIDI_shm_queue_t MPIDI_shm_recvq_posted = { NULL, NULL };
MPIDI_shm_queue_t MPIDI_shm_recvq_unexpected = { NULL, NULL };
