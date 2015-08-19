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
#ifndef NETMOD_AM_OFI_TYPES_H_INCLUDED
#define NETMOD_AM_OFI_TYPES_H_INCLUDED

#include "../ofi/types.h"
#include <mpidch4u.h>

EXTERN_C_BEGIN typedef enum {
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
    uint8_t payload[];
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

typedef struct {
    struct fi_context context;  /* fixed field, do not move */
    MPIDI_AM_OFI_hdr_t msg_hdr;
    uint8_t am_hdr[MPIDI_MAX_AM_HDR_SZ];
    MPIDI_OFI_lmt_msg_pyld_t lmt_info;
    uint64_t lmt_cntr;
    void *pack_buffer;
    MPIDI_netmod_am_completion_handler_fn cmpl_handler_fn;
} MPIDI_OFIAMReq_t;
#define AMREQ_OFI(req,field) ((MPIDI_OFIAMReq_t*)&MPIU_CH4U_REQUEST_AM_NETMOD(req))->field

#endif /* NETMOD_AM_OFI_TYPES_H_INCLUDED */
