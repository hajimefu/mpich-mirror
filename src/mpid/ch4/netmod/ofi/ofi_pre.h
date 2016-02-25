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

#ifndef NETMOD_OFI_PRE_H_INCLUDED
#define NETMOD_OFI_PRE_H_INCLUDED

#include <mpi.h>
#include "mpihandlemem.h"
#include <rdma/fabric.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_tagged.h>
#include <rdma/fi_rma.h>
#include <rdma/fi_atomic.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_errno.h>

/* Defines */

#define MPIDI_CH4_NMI_OFI_MAX_AM_HDR_SIZE    128
#define MPIDI_CH4_NMI_OFI_AM_HANDLER_ID_BITS   8
#define MPIDI_CH4_NMI_OFI_AM_TYPE_BITS         8
#define MPIDI_CH4_NMI_OFI_AM_HDR_SZ_BITS       8
#define MPIDI_CH4_NMI_OFI_AM_DATA_SZ_BITS     48
#define MPIDI_CH4_NMI_OFI_AM_CONTEXT_ID_BITS  24
#define MPIDI_CH4_NMI_OFI_AM_RANK_BITS        32
#define MPIDI_CH4_NMI_OFI_AM_MSG_HEADER_SIZE (sizeof(MPIDI_CH4_NMI_OFI_Am_header_t))

#ifdef MPIDI_CH4_NMI_OFI_CONFIG_USE_SCALABLE_ENDPOINTS
#define MPIDI_CH4_NMI_OFI_MAX_ENDPOINTS      256
#define MPIDI_CH4_NMI_OFI_MAX_ENDPOINTS_BITS   8
#else
#define MPIDI_CH4_NMI_OFI_MAX_ENDPOINTS        1
#define MPIDI_CH4_NMI_OFI_MAX_ENDPOINTS_BITS   0
#endif

/* Typedefs */

struct MPID_Comm;
struct MPID_Request;

#ifdef MPIDI_CH4_NMI_OFI_CONFIG_USE_SCALABLE_ENDPOINTS
typedef struct MPIDI_CH4_NMI_OFI_VCR {
    unsigned is_local : 1;
unsigned ep_idx   :
    MPIDI_CH4_NMI_OFI_MAX_ENDPOINTS_BITS;
unsigned addr_idx :
    (31 - MPIDI_CH4_NMI_OFI_MAX_ENDPOINTS_BITS);
} MPIDI_CH4_NMI_OFI_VCR;
#else
typedef struct MPIDI_CH4_NMI_OFI_VCR {
    unsigned is_local : 1;
    unsigned addr_idx : 31;
} MPIDI_CH4_NMI_OFI_VCR;
#endif

struct MPIDI_CH4_NMI_OFI_VCRT {
    MPIU_OBJECT_HEADER;
    unsigned              size;                /**< Number of entries in the table */
    MPIDI_CH4_NMI_OFI_VCR vcr_table[0];       /**< Array of virtual connection references */
};

typedef struct {
    struct MPIDI_CH4_NMI_OFI_VCRT *vcrt;
    struct MPIDI_CH4_NMI_OFI_VCRT *local_vcrt;
    void                          *huge_send_counters;
    void                          *huge_recv_counters;
    void                          *win_id_allocator;
    void                          *rma_id_allocator;
} MPIDI_CH4_NMI_OFI_Comm_t;
enum {
    MPIDI_AMTYPE_SHORT_HDR = 0,
    MPIDI_AMTYPE_SHORT,
    MPIDI_AMTYPE_LMT_REQ,
    MPIDI_AMTYPE_LMT_HDR_REQ,
    MPIDI_AMTYPE_LMT_ACK,
    MPIDI_AMTYPE_LONG_HDR_REQ,
    MPIDI_AMTYPE_LONG_HDR_ACK
};

typedef struct {
    uint64_t src_offset;
    uint64_t sreq_ptr;
    uint64_t am_hdr_src;
    uint64_t rma_key;
} MPIDI_CH4_NMI_OFI_Lmt_msg_payload_t;

typedef struct {
    uint64_t sreq_ptr;
} MPIDI_CH4_NMI_OFI_Ack_msg_payload_t;

typedef struct MPIDI_CH4_NMI_OFI_Am_reply_token_t {
    union {
        uint64_t val;
        struct {
            uint32_t context_id;
            uint32_t src_rank;
        } data;
    };
} MPIDI_CH4_NMI_OFI_Am_reply_token_t;

typedef struct MPIDI_CH4_NMI_OFI_Am_header_t {
uint64_t handler_id  :
    MPIDI_CH4_NMI_OFI_AM_HANDLER_ID_BITS;
uint64_t am_type     :
    MPIDI_CH4_NMI_OFI_AM_TYPE_BITS;
uint64_t am_hdr_sz   :
    MPIDI_CH4_NMI_OFI_AM_HDR_SZ_BITS;
uint64_t data_sz     :
    MPIDI_CH4_NMI_OFI_AM_DATA_SZ_BITS;
uint64_t context_id  :
    MPIDI_CH4_NMI_OFI_AM_CONTEXT_ID_BITS;
uint64_t src_rank    :
    MPIDI_CH4_NMI_OFI_AM_RANK_BITS;
    uint64_t payload[0];
} MPIDI_CH4_NMI_OFI_Am_header_t;

typedef struct {
    MPIDI_CH4_NMI_OFI_Am_header_t       hdr;
    MPIDI_CH4_NMI_OFI_Ack_msg_payload_t pyld;
} MPIDI_CH4_NMI_OFI_Ack_msg_t;

typedef struct {
    MPIDI_CH4_NMI_OFI_Am_header_t       hdr;
    MPIDI_CH4_NMI_OFI_Lmt_msg_payload_t pyld;
} MPIDI_CH4_NMI_OFI_Lmt_msg_t;

typedef struct {
    MPIDI_CH4_NMI_OFI_Lmt_msg_payload_t  lmt_info;
    uint64_t                             lmt_cntr;
    struct fid_mr                        *lmt_mr;
    union  {
        void                               *pack_buffer;
        MPIDI_CH4_NMI_OFI_Am_reply_token_t  reply_token;
    } clientdata;
    void                          *rreq_ptr;
    void                          *am_hdr;
    int (*cmpl_handler_fn)(struct MPID_Request *req);
    uint16_t                      am_hdr_sz;
    uint8_t                       pad[6];
    MPIDI_CH4_NMI_OFI_Am_header_t msg_hdr;
    uint8_t                       am_hdr_buf[MPIDI_CH4_NMI_OFI_MAX_AM_HDR_SIZE];
} MPIDI_CH4_NMI_OFI_Am_request_header_t;

typedef struct {
    struct fi_context                      context;  /* fixed field, do not move */
    int                                    event_id; /* fixed field, do not move */
    MPIDI_CH4_NMI_OFI_Am_request_header_t *req_hdr;
} MPIDI_CH4_NMI_OFI_Am_request_t;


typedef struct MPIDI_CH4_NMI_OFI_Noncontig_t{
    struct MPID_Segment  segment;
    char                 pack_buffer[0];
}MPIDI_CH4_NMI_OFI_Noncontig_t;

typedef struct {
    struct fi_context    context;  /* fixed field, do not move */
    int                  event_id; /* fixed field, do not move */
    int                  util_id;
    struct MPID_Comm    *util_comm;
    MPI_Datatype         datatype;
    MPIDI_CH4_NMI_OFI_Noncontig_t *noncontig;
    /* persistent send fields */
    union {
        struct {
            int   type;
            int   rank;
            int   tag;
            int   count;
            void *buf;
        } persist;
        struct iovec iov;
    } util;
} MPIDI_CH4_NMI_OFI_Request_t;

typedef struct {
    int index;
} MPIDI_CH4_NMI_OFI_Dt_t;

#endif
