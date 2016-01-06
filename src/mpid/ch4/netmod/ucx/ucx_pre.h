/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#ifndef UCX_PRE_H_INCLUDED
#define UCX_PRE_H_INCLUDED

#include <ucp/api/ucp.h>
#include "ucx_pre_common.h"

#define HAVE_MPIDI_CH4_NM_datatype_commit_hook
#define HAVE_MPIDI_CH4_NM_datatype_destroy_hook

typedef struct {
   int has_ucp;
   ucp_datatype_t ucp_datatype;
} MPIDI_UCX_dt_t;

typedef struct {
  ucp_tag_message_h  message_handler;
} MPIDI_UCX_request_t;

typedef struct {
    int handler_id;
    char *pack_buffer;
} MPIDI_UCX_am_request_t;

typedef struct MPIDI_UCX_am_header_t {
    uint64_t handler_id;
    uint64_t data_sz;
    uint64_t payload[0];
} MPIDI_UCX_am_header_t;

typedef struct MPIDI_UCX_win_info{
    ucp_rkey_h rkey;
    uint64_t addr;
    uint32_t disp;
} __attribute__ ((packed)) MPIDI_UCX_win_info_t ;


typedef struct {
    MPIDI_UCX_win_info_t *info_table;
    ucp_mem_h mem_h;
    int need_local_flush;
} MPIDI_UCX_win_t;

typedef struct {
    char addr[128];
} MPIDI_CH4_NMI_UCX_gpid_t;

typedef struct {
    ucp_ep_h dest;
} MPIDI_CH4_NMI_UCX_addr_t;

#endif /* UCX_PRE_H_INCLUDED */
