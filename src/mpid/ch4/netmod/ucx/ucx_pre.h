/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 */

#ifndef NETMOD_UCX_PRE_H_INCLUDED
#define NETMOD_UCX_PRE_H_INCLUDED

#include <mpi.h>

#include <ucp/api/ucp.h>
#include "mpihandlemem.h"
#include "ucx_pre_common.h"

struct MPID_Request;
struct MPID_Comm;
#define HAVE_MPIDI_CH4_NM_datatype_commit_hook
#define HAVE_MPIDI_CH4_NM_datatype_destroy_hook
typedef struct {
   int has_ucp;
   ucp_datatype_t ucp_datatype;
} MPIDI_CH4_NMI_UCX_Dt_t;


typedef struct {
  ucp_tag_message_h  message_handler;
} MPIDI_CH4_NMI_UCX_Request_t;


typedef struct {
    char *pack_buffer;
} MPIDI_CH4_NMI_UCX_Am_request_t;

typedef struct {
    struct MPID_Request* req;
} MPIDI_CH4_NMI_UCX_Ucp_request_t;

typedef struct MPIDI_CH4_NMI_UCX_Am_header_t {
    uint64_t handler_id;
    uint64_t data_sz;
    uint64_t context_id;
    uint64_t src_rank;
    uint64_t payload[0];
} MPIDI_CH4_NMI_UCX_Am_header_t;

typedef struct {
    union {
        uint64_t val;
        struct {
            uint32_t context_id;
            uint32_t src_rank;
        } data;
    };
} MPIDI_CH4_NMI_UCX_Am_reply_token_t;

#endif /* NETMOD_UCX_PRE_H_INCLUDED */
