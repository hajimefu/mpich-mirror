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
#ifndef MPIDCH4_TYPES_H_INCLUDED
#define MPIDCH4_TYPES_H_INCLUDED

#include <mpidimpl.h>
#include <stdio.h>
#include "mpich_cvars.h"
#include "pmi.h"

/* Macros and inlines */
/* match/ignore bit manipulation
 *
 * 0123 4567 01234567 0123 4567 01234567 0123 4567 01234567 01234567 01234567
 *     |                  |                  |
 * ^   |    context id    |       source     |       message tag
 * |   |                  |                  |
 * +---- protocol
 */
#define MPIDI_CH4U_PROTOCOL_MASK (0x9000000000000000ULL)
#define MPIDI_CH4U_CONTEXT_MASK  (0x0FFFF00000000000ULL)
#define MPIDI_CH4U_SOURCE_MASK   (0x00000FFFF0000000ULL)
#define MPIDI_CH4U_TAG_MASK      (0x000000000FFFFFFFULL)
#define MPIDI_CH4U_DYNPROC_SEND  (0x4000000000000000ULL)
#define MPIDI_CH4U_TAG_SHIFT     (28)
#define MPIDI_CH4U_SOURCE_SHIFT  (16)


#define MAX_PROGRESS_HOOKS 16
typedef int (*progress_func_ptr_t) (int *made_progress);

typedef enum {
    MPIDI_CH4U_AM_SEND = 0,
    MPIDI_CH4U_AM_SSEND_REQ,
    MPIDI_CH4U_AM_SSEND_ACK,
} MPIDI_CH4U_AM_TYPE;

typedef struct MPIDI_CH4U_AM_Hdr_t {
    uint64_t msg_tag;
} MPIDI_CH4U_AM_Hdr_t;

typedef struct MPIDI_CH4U_Ssend_req_msg_t {
    MPIDI_CH4U_AM_Hdr_t hdr;
    uint64_t sreq_ptr;
} MPIDI_CH4U_Ssend_req_msg_t;

typedef struct MPIDI_CH4U_Ssend_ack_msg_t {
    uint64_t sreq_ptr;
} MPIDI_CH4U_Ssend_ack_msg_t;

typedef struct MPIDI_CH4_Global_t {
    MPID_Request *request_test;
    MPID_Comm *comm_test;
    int pname_set;
    int pname_len;
    char pname[MPI_MAX_PROCESSOR_NAME];
    int is_initialized;
    MPID_Comm **comms;
    progress_func_ptr_t progress_hooks[MAX_PROGRESS_HOOKS];

#ifndef MPIDI_CH4U_USE_PER_COMM_QUEUE
    MPIDI_CH4U_Devreq_t *posted_list;
    MPIDI_CH4U_Devreq_t *unexp_list;
#endif

    void *netmod_context[];
} MPIDI_CH4_Global_t;
extern MPIDI_CH4_Global_t MPIDI_CH4_Global;

#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO

/* Define a data structure for CH4 to keep locality information for each
 * process. This is only used if the netmod doesn't explicitly turn it off. */
extern int *MPIDI_CH4U_gpid_local;      /* For now, this is implemented as a flat
                                         * integer array indexed by gpid. This will
                                         * probably be reimplemented with something more
                                         * efficient. */

#endif

#endif /* MPIDCH4_TYPES_H_INCLUDED */
