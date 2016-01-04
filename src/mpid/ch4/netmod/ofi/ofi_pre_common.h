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
    void      *huge_send_counters;
    void      *huge_recv_counters;
    void      *win_id_allocator;
    void      *rma_id_allocator;
} MPIDI_netmod_ofi_comm_t;

struct MPID_Comm;
struct MPID_Request;

typedef enum {
    MPIDI_AMTYPE_SHORT_HDR = 0,
    MPIDI_AMTYPE_SHORT,
    MPIDI_AMTYPE_LMT_REQ,
    MPIDI_AMTYPE_LMT_HDR_REQ,
    MPIDI_AMTYPE_LMT_ACK,
    MPIDI_AMTYPE_LONG_HDR_REQ,
    MPIDI_AMTYPE_LONG_HDR_ACK
} MPIDI_amtype;

typedef struct {
    uint64_t src_offset;
    uint64_t sreq_ptr;
    uint64_t am_hdr_src;
} MPIDI_OFI_lmt_msg_pyld_t;

typedef struct {
    uint64_t sreq_ptr;
} MPIDI_OFI_Ack_msg_pyld_t;

typedef struct MPIDI_AM_OFI_hdr_t {
    uint16_t handler_id;
    uint8_t am_type;
    uint8_t pad[5];
    uint64_t am_hdr_sz;
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

    void *pack_buffer;
    void *rreq_ptr;
    void *am_hdr;

    int (*cmpl_handler_fn) (struct MPID_Request * req);
    uint16_t am_hdr_sz;
    uint8_t pad[6];

    MPIDI_AM_OFI_hdr_t msg_hdr;
    uint8_t am_hdr_buf[MPIDI_MAX_AM_HDR_SZ];
} MPIDI_am_ofi_req_hdr_t;

typedef struct {
    struct fi_context  context;  /* fixed field, do not move */
    MPIDI_am_ofi_req_hdr_t *req_hdr;
} MPIDI_netmod_ofi_amrequest_t;

typedef MPIDI_netmod_ofi_comm_t MPIDI_netmod_am_ofi_comm_t;

#endif
