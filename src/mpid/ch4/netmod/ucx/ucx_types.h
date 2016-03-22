/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  *  (C) 2006 by Argonne National Laboratory.
 *   *      See COPYRIGHT in top-level directory.
 *    *
 *    */

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED
#include <ucp/api/ucp.h>
#include <ucp/api/ucp_def.h>
#include "mpiimpl.h"

#define __SHORT_FILE__                          \
  (strrchr(__FILE__,'/')                        \
   ? strrchr(__FILE__,'/')+1                    \
   : __FILE__                                   \
)

#define UCP_PEER_NAME_MAX         HOST_NAME_MAX
#define MPIDI_CH4_NMI_UCX_KVSAPPSTRLEN 1024

#define MPIDI_MAP_NOT_FOUND      ((void*)(-1UL))

/* Active Message Stuff */
#define MPIDI_CH4_NMI_UCX_NUM_AM_BUFFERS       (64)
#define MPIDI_CH4_NMI_UCX_MAX_AM_EAGER_SZ      (16*1024)
#define MPIDI_CH4_NMI_UCX_AM_TAG               (1 << 28)
#define MPIDI_CH4_NMI_UCX_MAX_AM_HANDLERS      (64)

#define MPIDI_CH4_NMI_UCX_BUF_POOL_SIZE            (1024)
#define MPIDI_CH4_NMI_UCX_BUF_POOL_NUM             (1024)

typedef struct {
    MPID_Request *req;
} MPIDI_CH4_NMI_UCX_Ucp_request_t;

typedef struct {
    ucp_context_h context;
    ucp_worker_h worker;
    char addrname[UCP_PEER_NAME_MAX];
    size_t addrname_len;
    MPID_Node_id_t *node_map;
    MPID_Node_id_t max_node_id;
    ucp_address_t *if_address;
    char kvsname[MPIDI_CH4_NMI_UCX_KVSAPPSTRLEN];
    char pname[MPI_MAX_PROCESSOR_NAME];
    struct iovec    am_iov[MPIDI_CH4_NMI_UCX_NUM_AM_BUFFERS];
    void           *am_bufs[MPIDI_CH4_NMI_UCX_NUM_AM_BUFFERS];
    MPIDI_CH4_NMI_UCX_Ucp_request_t *ucp_am_requests[MPIDI_CH4_NMI_UCX_NUM_AM_BUFFERS];
    MPIDI_CH4_NM_am_target_handler_fn am_handlers[MPIDI_CH4_NMI_UCX_MAX_AM_HANDLERS];
    MPIDI_CH4_NM_am_origin_handler_fn send_cmpl_handlers[MPIDI_CH4_NMI_UCX_MAX_AM_HANDLERS];
} MPIDI_CH4_NMI_UCX_Global_t;

extern MPIDI_CH4_NMI_UCX_Global_t MPIDI_CH4_NMI_UCX_Global;
extern ucp_ep_h *MPIDI_CH4_NMI_UCX_eps;


/* UCX TAG Layout */

/* 01234567 01234567 01234567 01234567 01234567 01234567 01234567 01234567
 *  context_id (16) |source rank (16) | Message Tag (32)+ERROR BITS
 */

#define MPIDI_CH4_NMI_UCX_CONTEXT_TAG_BITS 16
#define MPIDI_CH4_NMI_UCX_CONTEXT_RANK_BITS 16
#define UCX_TAG_BITS 32

#define MPIDI_CH4_NMI_UCX_TAG_MASK      (0x00000000FFFFFFFFULL)
#define MPIDI_CH4_NMI_UCX_SOURCE_MASK   (0x0000FFFF00000000ULL)
#define MPIDI_CH4_NMI_UCX_TAG_SHIFT     (32)
#define MPIDI_CH4_NMI_UCX_SOURCE_SHIFT  (16)

#endif /* TYPES_H_INCLUDED */
