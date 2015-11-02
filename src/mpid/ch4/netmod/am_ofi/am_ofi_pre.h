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

#ifndef NETMOD_AM_OFI_PRE_H_INCLUDED
#define NETMOD_AM_OFI_PRE_H_INCLUDED

#include <mpi.h>
#include <rdma/fabric.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_tagged.h>
#include <rdma/fi_rma.h>
#include <rdma/fi_atomic.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_errno.h>
#include "../ofi/ofi_pre_common.h"

struct MPID_Comm;
struct MPID_Request;

typedef enum {
    MPIDI_AMTYPE_SHORT_HDR = 0,
    MPIDI_AMTYPE_SHORT,
    MPIDI_AMTYPE_LMT_REQ,
    MPIDI_AMTYPE_LMT_ACK,
} MPIDI_amtype;

typedef struct {
    uint64_t src_offset;
    uint64_t sreq_ptr;
} MPIDI_OFI_lmt_msg_pyld_t;

typedef struct {
    uint64_t sreq_ptr;
} MPIDI_OFI_Ack_msg_pyld_t;

typedef struct MPIDI_AM_OFI_hdr_t {
    uint16_t handler_id;
    uint16_t am_hdr_sz;
    uint8_t am_type;
    uint8_t pad[3];
    uint64_t data_sz;
    uint8_t payload[0];
} MPIDI_AM_OFI_hdr_t;
#define MPIDI_AM_OFI_MSG_HDR_SZ (sizeof(MPIDI_AM_OFI_hdr_t))

typedef struct {
    MPIDI_AM_OFI_hdr_t hdr;
    MPIDI_OFI_Ack_msg_pyld_t pyld;
} MPIDI_OFI_Ack_msg_t;

typedef struct {
    MPIDI_AM_OFI_hdr_t hdr;
    MPIDI_OFI_lmt_msg_pyld_t pyld;
} MPIDI_OFI_lmt_msg_t;

#ifndef MPIDI_MAX_AM_HDR_SZ
#define MPIDI_MAX_AM_HDR_SZ 128
#endif

typedef struct {
    MPIDI_OFI_lmt_msg_pyld_t lmt_info;
    uint64_t lmt_cntr;

    union {
        void *pack_buffer;
        void *rreq_ptr;
    };

    int (*cmpl_handler_fn) (struct MPID_Request * req);
    void *am_hdr;
    MPIDI_AM_OFI_hdr_t msg_hdr;
    uint8_t am_hdr_buf[MPIDI_MAX_AM_HDR_SZ];
} MPIDI_am_ofi_req_hdr_t;

typedef struct {
    struct fi_context  context;  /* fixed field, do not move */
    MPIDI_am_ofi_req_hdr_t *req_hdr;
} MPIDI_netmod_am_ofi_amrequest_t;

typedef struct {
    int dummy;
} MPIDI_netmod_am_ofi_request_t;

typedef MPIDI_netmod_ofi_comm_t MPIDI_netmod_am_ofi_comm_t;

#endif
