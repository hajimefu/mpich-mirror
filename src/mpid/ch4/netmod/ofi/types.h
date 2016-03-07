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
#ifndef NETMOD_OFI_TYPES_H_INCLUDED
#define NETMOD_OFI_TYPES_H_INCLUDED

#include <netdb.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdint.h>
#include "ofi_pre.h"
#include "ch4_types.h"
#include "mpidch4r.h"
#include "fi_list.h"

EXTERN_C_BEGIN
#define __SHORT_FILE__                          \
    (strrchr(__FILE__,'/')                      \
     ? strrchr(__FILE__,'/')+1                  \
     : __FILE__                                 \
        )
#define MPIDI_CH4_NMI_OFI_MAP_NOT_FOUND            ((void*)(-1UL))
#define MPIDI_CH4_NMI_OFI_MAJOR_VERSION            1
#define MPIDI_CH4_NMI_OFI_MINOR_VERSION            0
#define MPIDI_CH4_NMI_OFI_DEFAULT_SHORT_SEND_SIZE  (16 * 1024)
#define MPIDI_CH4_NMI_OFI_NUM_AM_BUFFERS           (8)
#define MPIDI_CH4_NMI_OFI_AM_BUFF_SZ               (1 * 1024 * 1024)
#define MPIDI_CH4_NMI_OFI_CACHELINE_SIZE           (64)
#define MPIDI_CH4_NMI_OFI_IOV_MAX                  (32)
#define MPIDI_CH4_NMI_OFI_BUF_POOL_SIZE            (1024)
#define MPIDI_CH4_NMI_OFI_BUF_POOL_NUM             (1024)
#define MPIDI_CH4_NMI_OFI_NUM_CQ_BUFFERED          (1024)
#define MPIDI_CH4_NMI_OFI_MAX_AM_HANDLERS_TOTAL    (24)
#define MPIDI_CH4_NMI_OFI_INTERNAL_HANDLER_CONTROL (MPIDI_CH4_NMI_OFI_MAX_AM_HANDLERS_TOTAL-1)
#define MPIDI_CH4_NMI_OFI_INTERNAL_HANDLER_NEXT    (MPIDI_CH4_NMI_OFI_MAX_AM_HANDLERS_TOTAL-2)
#define MPIDI_CH4_NMI_OFI_MAX_AM_HANDLERS          (MPIDI_CH4_NMI_OFI_INTERNAL_HANDLER_NEXT-1)

#ifdef USE_OFI_TAGGED
#define MPIDI_CH4_NMI_OFI_ENABLE_TAGGED          1
#define MPIDI_CH4_NMI_OFI_ENABLE_AM              1
#define MPIDI_CH4_NMI_OFI_ENABLE_RMA             1
#else
#define MPIDI_CH4_NMI_OFI_ENABLE_TAGGED          0
#define MPIDI_CH4_NMI_OFI_ENABLE_AM              1
#define MPIDI_CH4_NMI_OFI_ENABLE_RMA             1
#endif

#ifdef MPIDI_CH4_NMI_OFI_CONFIG_USE_SCALABLE_ENDPOINTS
#define MPIDI_CH4_NMI_OFI_ENABLE_SCALABLE_ENDPOINTS 1
#else
#define MPIDI_CH4_NMI_OFI_ENABLE_SCALABLE_ENDPOINTS 0
#endif

#ifdef MPIDI_CH4_NMI_OFI_CONFIG_USE_AV_TABLE
#define MPIDI_CH4_NMI_OFI_ENABLE_AV_TABLE 1
#else
#define MPIDI_CH4_NMI_OFI_ENABLE_AV_TABLE 0
#endif

/* match/ignore bit manipulation
 *
 * 0123 4567 01234567 0123 4567 01234567 0123 4567 01234567 01234567 01234567
 *     |                  |                  |
 * ^   |    context id    |       source     |       message tag
 * |   |                  |                  |
 * +---- protocol
 */
#define MPIDI_CH4_NMI_OFI_PROTOCOL_MASK (0x9000000000000000ULL)
#define MPIDI_CH4_NMI_OFI_CONTEXT_MASK  (0x0FFFF00000000000ULL)
#define MPIDI_CH4_NMI_OFI_SOURCE_MASK   (0x00000FFFF0000000ULL)
#define MPIDI_CH4_NMI_OFI_TAG_MASK      (0x000000000FFFFFFFULL)
#define MPIDI_CH4_NMI_OFI_SYNC_SEND     (0x1000000000000000ULL)
#define MPIDI_CH4_NMI_OFI_SYNC_SEND_ACK (0x2000000000000000ULL)
#define MPIDI_CH4_NMI_OFI_DYNPROC_SEND  (0x4000000000000000ULL)
#define MPIDI_CH4_NMI_OFI_TAG_SHIFT     (28)
#define MPIDI_CH4_NMI_OFI_SOURCE_SHIFT  (16)

/* RMA Key Space division
 *    |                  |                  |                    |
 *    ...     Context ID |   Huge RMA       |  Window Instance   |
 *    |                  |                  |                    |
 */
/* 64-bit key space                         */
/* 2M  window instances per comm           */
/* 2M  outstanding huge RMAS per comm      */
/* 4M  communicators                       */
#define MPIDI_CH4_NMI_OFI_MAX_WINDOWS_BITS_64  (21)
#define MPIDI_CH4_NMI_OFI_MAX_HUGE_RMA_BITS_64 (21)
#define MPIDI_CH4_NMI_OFI_MAX_HUGE_RMAS_64     (1<<(MPIDI_CH4_NMI_OFI_MAX_HUGE_RMA_BITS_64))
#define MPIDI_CH4_NMI_OFI_MAX_WINDOWS_64       (1<<(MPIDI_CH4_NMI_OFI_MAX_WINDOWS_BITS_64))
#define MPIDI_CH4_NMI_OFI_HUGE_RMA_SHIFT_64    (MPIDI_CH4_NMI_OFI_MAX_WINDOWS_BITS_64)
#define MPIDI_CH4_NMI_OFI_CONTEXT_SHIFT_64     (MPIDI_CH4_NMI_OFI_MAX_WINDOWS_BITS_64+MPIDI_CH4_NMI_OFI_MAX_HUGE_RMA_BITS_64)

/* 32-bit key space                         */
/* 4096 window instances per comm           */
/* 256  outstanding huge RMAS per comm      */
/* 4096 communicators                       */
#define MPIDI_CH4_NMI_OFI_MAX_WINDOWS_BITS_32  (12)
#define MPIDI_CH4_NMI_OFI_MAX_HUGE_RMA_BITS_32 (8)
#define MPIDI_CH4_NMI_OFI_MAX_HUGE_RMAS_32     (1<<(MPIDI_CH4_NMI_OFI_MAX_HUGE_RMA_BITS_32))
#define MPIDI_CH4_NMI_OFI_MAX_WINDOWS_32       (1<<(MPIDI_CH4_NMI_OFI_MAX_WINDOWS_BITS_32))
#define MPIDI_CH4_NMI_OFI_HUGE_RMA_SHIFT_32    (MPIDI_CH4_NMI_OFI_MAX_WINDOWS_BITS_32)
#define MPIDI_CH4_NMI_OFI_CONTEXT_SHIFT_32     (MPIDI_CH4_NMI_OFI_MAX_WINDOWS_BITS_32+MPIDI_CH4_NMI_OFI_MAX_HUGE_RMA_BITS_32)

/* 16-bit key space                         */
/* 64 window instances per comm             */
/* 16 outstanding huge RMAS per comm        */
/* 64 communicators                          */
#define MPIDI_CH4_NMI_OFI_MAX_WINDOWS_BITS_16  (6)
#define MPIDI_CH4_NMI_OFI_MAX_HUGE_RMA_BITS_16 (4)
#define MPIDI_CH4_NMI_OFI_MAX_HUGE_RMAS_16     (1<<(MPIDI_CH4_NMI_OFI_MAX_HUGE_RMA_BITS_16))
#define MPIDI_CH4_NMI_OFI_MAX_WINDOWS_16       (1<<(MPIDI_CH4_NMI_OFI_MAX_WINDOWS_BITS_16))
#define MPIDI_CH4_NMI_OFI_HUGE_RMA_SHIFT_16    (MPIDI_CH4_NMI_OFI_MAX_WINDOWS_BITS_16)
#define MPIDI_CH4_NMI_OFI_CONTEXT_SHIFT_16     (MPIDI_CH4_NMI_OFI_MAX_WINDOWS_BITS_16+MPIDI_CH4_NMI_OFI_MAX_HUGE_RMA_BITS_16)

#define MPIDI_CH4_NMI_OFI_DT_SIZES 62
#define MPIDI_CH4_NMI_OFI_OP_SIZES 15

#define MPIDI_CH4_NMI_OFI_API_TAG 0
#define MPIDI_CH4_NMI_OFI_API_RMA 1
#define MPIDI_CH4_NMI_OFI_API_MSG 2
#define MPIDI_CH4_NMI_OFI_API_CTR 3

#define MPIDI_CH4_NMI_OFI_THREAD_UTIL_MUTEX     MPIDI_Global.mutexes[0].m
#define MPIDI_CH4_NMI_OFI_THREAD_PROGRESS_MUTEX MPIDI_Global.mutexes[1].m
#define MPIDI_CH4_NMI_OFI_THREAD_FI_MUTEX       MPIDI_Global.mutexes[2].m
#define MPIDI_CH4_NMI_OFI_THREAD_SPAWN_MUTEX    MPIDI_Global.mutexes[3].m

/* Field accessor macros */
#define MPIDI_CH4_NMI_OFI_GPID(req)                ((MPIDI_CH4_NMI_OFI_Gpid_t*)(req)->dev.pad)
#define MPIDI_CH4_NMI_OFI_OBJECT_HEADER_SIZE       offsetof(MPIDI_CH4_NMI_OFI_Offset_checker_t,  pad)
#define MPIDI_CH4_NMI_OFI_WIN(win)                 ((MPIDI_CH4_NMI_OFI_Win_t*)(win)->dev.pad)
#define MPIDI_CH4_NMI_OFI_AMREQUEST(req,field)     ((req)->dev.ch4.ch4r.netmod_am.ofi.field)
#define MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(req,field) ((req)->dev.ch4.ch4r.netmod_am.ofi.req_hdr->field)
#define MPIDI_CH4_NMI_OFI_AMREQUEST_HDR_PTR(req)   ((req)->dev.ch4.ch4r.netmod_am.ofi.req_hdr)


#define MPIDI_CH4_NMI_OFI_DATATYPE(dt)   ((dt)->dev.netmod.ofi)
#define MPIDI_CH4_NMI_OFI_COMM(comm)     ((comm)->dev.ch4.netmod.ofi)

#ifdef MPIDI_CH4_NMI_OFI_CONFIG_USE_SCALABLE_ENDPOINTS
#define MPIDI_CH4_NMI_OFI_COMM_TO_EP(comm,rank)  MPIDI_CH4_NMI_OFI_COMM(comm).vcrt->vcr_table[rank].ep_idx
#define MPIDI_CH4_NMI_OFI_EP_TX_TAG(x) MPIDI_Global.ctx[x].tx_tag
#define MPIDI_CH4_NMI_OFI_EP_TX_RMA(x) MPIDI_Global.ctx[x].tx_rma
#define MPIDI_CH4_NMI_OFI_EP_TX_MSG(x) MPIDI_Global.ctx[x].tx_msg
#define MPIDI_CH4_NMI_OFI_EP_TX_CTR(x) MPIDI_Global.ctx[x].tx_ctr
#define MPIDI_CH4_NMI_OFI_EP_RX_TAG(x) MPIDI_Global.ctx[x].rx_tag
#define MPIDI_CH4_NMI_OFI_EP_RX_RMA(x) MPIDI_Global.ctx[x].rx_rma
#define MPIDI_CH4_NMI_OFI_EP_RX_MSG(x) MPIDI_Global.ctx[x].rx_msg
#define MPIDI_CH4_NMI_OFI_EP_RX_CTR(x) MPIDI_Global.ctx[x].rx_ctr
#else
#define MPIDI_CH4_NMI_OFI_COMM_TO_EP(comm,rank) 0
#define MPIDI_CH4_NMI_OFI_EP_TX_TAG(x) MPIDI_Global.ep
#define MPIDI_CH4_NMI_OFI_EP_TX_RMA(x) MPIDI_Global.ep
#define MPIDI_CH4_NMI_OFI_EP_TX_MSG(x) MPIDI_Global.ep
#define MPIDI_CH4_NMI_OFI_EP_TX_CTR(x) MPIDI_Global.ep
#define MPIDI_CH4_NMI_OFI_EP_RX_TAG(x) MPIDI_Global.ep
#define MPIDI_CH4_NMI_OFI_EP_RX_RMA(x) MPIDI_Global.ep
#define MPIDI_CH4_NMI_OFI_EP_RX_MSG(x) MPIDI_Global.ep
#define MPIDI_CH4_NMI_OFI_EP_RX_CTR(x) MPIDI_Global.ep
#endif

#define MPIDI_CH4_NMI_OFI_NUM_CQ_ENTRIES 8

/* Typedefs */
typedef enum {
    MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_RAR = 1,
    MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_RAW = 2,
    MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_WAR = 4,
    MPIDI_CH4_NMI_OFI_ACCUMULATE_ORDER_WAW = 8
} MPIDI_CH4_NMI_OFI_Win_info_accumulate_ordering_t;

typedef enum {
    MPIDI_CH4_NMI_OFI_ACCUMULATE_SAME_OP,
    MPIDI_CH4_NMI_OFI_ACCUMULATE_SAME_OP_NO_OP
} MPIDI_CH4_NMI_OFI_Win_info_accumulate_ops_t;

enum {
    MPIDI_CH4_NMI_OFI_CTRL_ASSERT,    /**< Lock acknowledge      */
    MPIDI_CH4_NMI_OFI_CTRL_LOCKACK,   /**< Lock acknowledge      */
    MPIDI_CH4_NMI_OFI_CTRL_LOCKALLACK,/**< Lock all acknowledge  */
    MPIDI_CH4_NMI_OFI_CTRL_LOCKREQ,   /**< Lock window           */
    MPIDI_CH4_NMI_OFI_CTRL_LOCKALLREQ,/**< Lock all window       */
    MPIDI_CH4_NMI_OFI_CTRL_UNLOCK,    /**< Unlock window         */
    MPIDI_CH4_NMI_OFI_CTRL_UNLOCKACK, /**< Unlock window         */
    MPIDI_CH4_NMI_OFI_CTRL_UNLOCKALL, /**< Unlock window         */
    MPIDI_CH4_NMI_OFI_CTRL_UNLOCKALLACK,
    /**< Unlock window         */
    MPIDI_CH4_NMI_OFI_CTRL_COMPLETE,  /**< End a START epoch     */
    MPIDI_CH4_NMI_OFI_CTRL_POST,      /**< Begin POST epoch      */
    MPIDI_CH4_NMI_OFI_CTRL_HUGE,      /**< Huge message          */
    MPIDI_CH4_NMI_OFI_CTRL_HUGEACK,   /**< Huge message ack      */
    MPIDI_CH4_NMI_OFI_CTRL_HUGE_CLEANUP
    /**< Huge message cleanup  */
};

enum {
    MPIDI_CH4_NMI_OFI_EVENT_ABORT,
    MPIDI_CH4_NMI_OFI_EVENT_SEND,
    MPIDI_CH4_NMI_OFI_EVENT_RECV,
    MPIDI_CH4_NMI_OFI_EVENT_RMA_DONE,
    MPIDI_CH4_NMI_OFI_EVENT_AM_SEND,
    MPIDI_CH4_NMI_OFI_EVENT_AM_RECV,
    MPIDI_CH4_NMI_OFI_EVENT_AM_READ,
    MPIDI_CH4_NMI_OFI_EVENT_AM_MULTI,
    MPIDI_CH4_NMI_OFI_EVENT_PEEK,
    MPIDI_CH4_NMI_OFI_EVENT_RECV_HUGE,
    MPIDI_CH4_NMI_OFI_EVENT_SEND_HUGE,
    MPIDI_CH4_NMI_OFI_EVENT_SSEND_ACK,
    MPIDI_CH4_NMI_OFI_EVENT_GET_HUGE,
    MPIDI_CH4_NMI_OFI_EVENT_CHUNK_DONE,
    MPIDI_CH4_NMI_OFI_EVENT_INJECT_EMU,
    MPIDI_CH4_NMI_OFI_EVENT_DYNPROC_DONE,
    MPIDI_CH4_NMI_OFI_EVENT_ACCEPT_PROBE
};

enum {
    MPIDI_CH4_NMI_OFI_REQUEST_LOCK,
    MPIDI_CH4_NMI_OFI_REQUEST_LOCKALL
};

enum {
    MPIDI_CH4_NMI_OFI_PEEK_START,
    MPIDI_CH4_NMI_OFI_PEEK_NOT_FOUND,
    MPIDI_CH4_NMI_OFI_PEEK_FOUND
};

typedef struct {
    fi_addr_t dest;
} MPIDI_CH4_NMI_OFI_Addr_entry_t;

typedef struct {
    int                            size;
    MPIDI_CH4_NMI_OFI_Addr_entry_t table[0];/**< Array of physical addresses */
} MPIDI_CH4_NMI_OFI_Addr_table_t;

typedef struct {
    char              pad[MPIDI_REQUEST_HDR_SIZE];
    struct fi_context context;          /* fixed field, do not move */
    int               event_id;         /* fixed field, do not move */
    int               index;
} MPIDI_CH4_NMI_OFI_Am_repost_request_t;

typedef struct {
    char               pad[MPIDI_REQUEST_HDR_SIZE];
    struct fi_context  context;          /* fixed field, do not move */
    int                event_id;         /* fixed field, do not move */
    MPID_Request      *signal_req;
} MPIDI_CH4_NMI_OFI_Ssendack_request_t;

typedef struct {
    char pad[MPIDI_REQUEST_HDR_SIZE];
    struct fi_context context;          /* fixed field, do not move */
    int               event_id;         /* fixed field, do not move */
    int               done;
    uint32_t          tag;
    uint32_t          source;
    uint64_t          msglen;
} MPIDI_CH4_NMI_OFI_Dynamic_process_request_t;

typedef struct {
    uint8_t  op;
    uint8_t  dt;
    unsigned atomic_valid:2;
    unsigned fetch_atomic_valid:2;
    unsigned compare_atomic_valid:2;
    unsigned dtsize:10;
    uint64_t max_atomic_count;
    uint64_t max_compare_atomic_count;
    uint64_t max_fetch_atomic_count;
} MPIDI_CH4_NMI_OFI_Atomic_valid_t;

typedef struct {
    struct fid_ep *tx_tag;
    struct fid_ep *rx_tag;

    struct fid_ep *tx_rma;
    struct fid_ep *rx_rma;

    struct fid_ep *tx_msg;
    struct fid_ep *rx_msg;

    struct fid_ep *tx_ctr;
    struct fid_ep *rx_ctr;

    int ctx_offset;
} MPIDI_CH4_NMI_OFI_Context_t;

typedef union {
    MPID_Thread_mutex_t m;
    char                cacheline[MPIDI_CH4_NMI_OFI_CACHELINE_SIZE];
} MPIDI_CH4_NMI_OFI_Cacheline_mutex_t __attribute__((aligned(MPIDI_CH4_NMI_OFI_CACHELINE_SIZE)));

typedef struct {
    struct fi_cq_tagged_entry  cq_entry;
    fi_addr_t                  source;
    struct slist_entry         entry;
} MPIDI_CH4_NMI_OFI_Cq_list_t;

typedef struct  {
    struct fi_cq_tagged_entry cq_entry;
} MPIDI_CH4_NMI_OFI_Cq_buff_entry_t;

/* Global state data */
#define MPIDI_KVSAPPSTRLEN 1024
typedef struct {
    /* OFI objects */
    struct fid_domain *domain;
    struct fid_fabric *fabric;
    struct fid_av     *av;
    struct fid_ep     *ep;
    struct fid_cq     *p2p_cq;
    struct fid_cntr   *rma_ctr;

    /* Queryable limits */
    uint64_t        max_buffered_send;
    uint64_t        max_buffered_write;
    uint64_t        max_send;
    uint64_t        max_write;
    uint64_t        max_short_send;
    uint64_t        max_mr_key_size;
    int             max_windows_bits;
    int             max_huge_rma_bits;
    int             max_huge_rmas;
    int             huge_rma_shift;
    int             context_shift;
    size_t          iov_limit;
    MPID_Node_id_t *node_map;
    MPID_Node_id_t  max_node_id;

    /* Mutexex and endpoints */
    MPIDI_CH4_NMI_OFI_Cacheline_mutex_t mutexes[4];
    MPIDI_CH4_NMI_OFI_Context_t         ctx[MPIDI_CH4_NMI_OFI_MAX_ENDPOINTS];

    /* Window/RMA Globals */
    void                             *win_map;
    uint64_t                          cntr;
    MPIDI_CH4_NMI_OFI_Atomic_valid_t  win_op_table[MPIDI_CH4_NMI_OFI_DT_SIZES][MPIDI_CH4_NMI_OFI_OP_SIZES];

    /* Active Message Globals */
    struct iovec                           am_iov[MPIDI_CH4_NMI_OFI_NUM_AM_BUFFERS];
    struct fi_msg                          am_msg[MPIDI_CH4_NMI_OFI_NUM_AM_BUFFERS];
    void                                  *am_bufs[MPIDI_CH4_NMI_OFI_NUM_AM_BUFFERS];
    MPIDI_CH4_NMI_OFI_Am_repost_request_t  am_reqs[MPIDI_CH4_NMI_OFI_NUM_AM_BUFFERS];
    MPIDI_CH4_NM_am_target_handler_fn      am_handlers[MPIDI_CH4_NMI_OFI_MAX_AM_HANDLERS_TOTAL];
    MPIDI_CH4_NM_am_origin_handler_fn      am_send_cmpl_handlers[MPIDI_CH4_NMI_OFI_MAX_AM_HANDLERS_TOTAL];
    MPIU_buf_pool_t                       *am_buf_pool;
    OPA_int_t                              am_inflight_inject_emus;

    /* Completion queue buffering */
    MPIDI_CH4_NMI_OFI_Cq_buff_entry_t cq_buffered[MPIDI_CH4_NMI_OFI_NUM_CQ_BUFFERED];
    struct slist                      cq_buff_list;
    int                               cq_buff_head;
    int                               cq_buff_tail;

    /* Process management and PMI globals */
    int    pname_set;
    int    pname_len;
    int    jobid;
    char   addrname[FI_NAME_MAX];
    size_t addrnamelen;
    char   kvsname[MPIDI_KVSAPPSTRLEN];
    char   pname[MPI_MAX_PROCESSOR_NAME];
    int    port_name_tag_mask[MPIR_MAX_CONTEXT_MASK];
} MPIDI_CH4_NMI_OFI_Global_t;

typedef struct {
    char           addr[62];
    MPID_Node_id_t node;
} MPIDI_CH4_NMI_OFI_Gpid_t;

typedef struct {
    uint32_t index;
} MPIDI_CH4_NMI_OFI_Datatype_t;
/* These control structures have to be the same size */
typedef struct {
    int16_t  type;
    int16_t  lock_type;
    int      origin_rank;
    uint64_t win_id;
    int      dummy[8];
} MPIDI_CH4_NMI_OFI_Win_control_t;

typedef struct {
    int16_t       type;
    int16_t       seqno;
    int           origin_rank;
    MPID_Request *ackreq;
    char         *send_buf;
    size_t        msgsize;
    int           comm_id;
    int           endpoint_id;
    int           rma_key;
} MPIDI_CH4_NMI_OFI_Send_control_t;

typedef struct {
    void *addr;
    void *result_addr;
    void *req;
    MPID_Win *win;
    MPI_Datatype type;
    MPI_Op op;
    int    origin_endpoint;
    size_t len;
} MPIDI_CH4_NMI_OFI_Msg_info_t;

typedef struct {
    MPIU_OBJECT_HEADER;
    void *pad;
} MPIDI_CH4_NMI_OFI_Offset_checker_t;

typedef struct {
    uintptr_t  target_base_addr;
    uintptr_t  origin_base_addr;
    uintptr_t  result_base_addr;
    size_t     target_count;
    size_t     origin_count;
    size_t     result_count;
    struct iovec   *target_iov;
    struct iovec   *origin_iov;
    struct iovec   *result_iov;
    size_t     target_idx;
    uintptr_t  target_addr;
    uintptr_t  target_size;
    size_t     origin_idx;
    uintptr_t  origin_addr;
    uintptr_t  origin_size;
    size_t     result_idx;
    uintptr_t  result_addr;
    uintptr_t  result_size;
    size_t     buf_limit;
    size_t     buf_limit_left;
} MPIDI_CH4_NMI_OFI_Iovec_state_t;

typedef struct {
    MPID_Datatype  *pointer;
    MPI_Datatype    type;
    int             count;
    int             contig;
    MPI_Aint        true_lb;
    size_t  size;
    int             num_contig;
    DLOOP_VECTOR   *map;
    DLOOP_VECTOR    __map;
} MPIDI_CH4_NMI_OFI_Win_datatype_t;

typedef struct {
    char                      pad[MPIDI_REQUEST_HDR_SIZE];
    struct fi_context         context;   /* fixed field, do not move */
    int                       event_id;  /* fixed field, do not move */
    struct MPIDI_Iovec_array *next;
    union {
        struct {
            struct iovec      *originv;
            struct fi_rma_iov *targetv;
        } put_get;
        struct {
            struct fi_ioc     *originv;
            struct fi_rma_ioc *targetv;
            struct fi_ioc     *resultv;
            struct fi_ioc     *comparev;
        } cas;
        struct {
            struct fi_ioc     *originv;
            struct fi_rma_ioc *targetv;
        } accumulate;
        struct {
            struct fi_ioc     *originv;
            struct fi_rma_ioc *targetv;
            struct fi_ioc     *resultv;
        } get_accumulate;
    } iov;
    char iov_store[0]; /* Flexible array, do not move */
} MPIDI_CH4_NMI_OFI_Iovec_array_t;

typedef struct {
    MPIDI_CH4_NMI_OFI_Iovec_state_t  iovs;
    MPIDI_CH4_NMI_OFI_Win_datatype_t origin_dt;
    MPIDI_CH4_NMI_OFI_Win_datatype_t target_dt;
    MPIDI_CH4_NMI_OFI_Win_datatype_t result_dt;
    MPIDI_CH4_NMI_OFI_Iovec_array_t  buf; /* Do not move me, flexible array */
} MPIDI_CH4_NMI_OFI_Win_noncontig_t;

typedef struct MPIDI_CH4_NMI_OFI_Win_request {
    MPIU_OBJECT_HEADER;
    char pad[MPIDI_REQUEST_HDR_SIZE - MPIDI_CH4_NMI_OFI_OBJECT_HEADER_SIZE];
    struct fi_context                     context;          /* fixed field, do not move */
    int                                   event_id;         /* fixed field, do not move */
    struct MPIDI_CH4_NMI_OFI_Win_request *next;
    int                                   target_rank;
    MPIDI_CH4_NMI_OFI_Win_noncontig_t    *noncontig;
} MPIDI_CH4_NMI_OFI_Win_request_t;

typedef struct {
    char pad[MPIDI_REQUEST_HDR_SIZE];
    struct fi_context  context;          /* fixed field, do not move */
    int                event_id;         /* fixed field, do not move */
    MPID_Request      *parent;           /* Parent request           */
} MPIDI_CH4_NMI_OFI_Chunk_request;

typedef struct {
    char pad[MPIDI_REQUEST_HDR_SIZE];
    struct fi_context                 context;          /* fixed field, do not move */
    int                               event_id;         /* fixed field, do not move */
    int (*done_fn)(struct fi_cq_tagged_entry *wc,
                   MPID_Request              *req);
    MPIDI_CH4_NMI_OFI_Send_control_t  remote_info;
    size_t                            cur_offset;
    MPID_Comm                        *comm_ptr;
    MPID_Request                     *localreq;
    struct fi_cq_tagged_entry         wc;
} MPIDI_CH4_NMI_OFI_Huge_chunk_t;

typedef struct {
    uint16_t  seqno;
    void     *chunk_q;
} MPIDI_CH4_NMI_OFI_Huge_recv_t;

typedef struct MPIDI_CH4_NMI_OFI_Huge_counter_t {
    uint16_t       counter;
    uint16_t       outstanding;
    struct fid_mr *mr;
} MPIDI_CH4_NMI_OFI_Huge_counter_t;

typedef struct MPIDI_CH4_NMI_OFI_Win_info_args_t {
    int                                              no_locks;
    MPIDI_CH4_NMI_OFI_Win_info_accumulate_ordering_t accumulate_ordering;
    MPIDI_CH4_NMI_OFI_Win_info_accumulate_ops_t      accumulate_ops;
    int                                              same_size;
    int                                              alloc_shared_noncontig;
} MPIDI_CH4_NMI_OFI_Win_info_args_t;

typedef struct {
    struct fid_mr                     *mr;
    uint64_t                           mr_key;
    uint64_t                           win_id;
    void                              *mmap_addr;
    int64_t                            mmap_sz;
    MPIDI_CH4_NMI_OFI_Win_request_t   *syncQ;
    void                              *msgQ;
    int                                count;
    MPIDI_CH4_NMI_OFI_Win_info_args_t  info_args;
} MPIDI_CH4_NMI_OFI_Win_t;

/* Externs */
extern MPIDI_CH4_NMI_OFI_Addr_table_t  *MPIDI_Addr_table;
extern MPIDI_CH4_NMI_OFI_Global_t       MPIDI_Global;
extern MPIU_Object_alloc_t              MPIDI_Request_mem;
extern MPID_Request                     MPIDI_Request_direct[];
extern int                              MPIR_Datatype_init_names(void);

EXTERN_C_END
#endif /* NETMOD_OFI_IMPL_H_INCLUDED */
