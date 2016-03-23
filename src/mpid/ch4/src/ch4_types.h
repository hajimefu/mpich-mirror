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
#ifndef CH4_TYPES_H_INCLUDED
#define CH4_TYPES_H_INCLUDED

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
#define MPIDI_CH4R_PROTOCOL_MASK (0x9000000000000000ULL)
#define MPIDI_CH4R_CONTEXT_MASK  (0x0FFFF00000000000ULL)
#define MPIDI_CH4R_SOURCE_MASK   (0x00000FFFF0000000ULL)
#define MPIDI_CH4R_TAG_MASK      (0x000000000FFFFFFFULL)
#define MPIDI_CH4R_DYNPROC_SEND  (0x4000000000000000ULL)
#define MPIDI_CH4R_TAG_SHIFT     (28)
#define MPIDI_CH4R_SOURCE_SHIFT  (16)
#define MPIDI_CH4R_SOURCE_SHIFT_UNPACK (sizeof(int)*8 - MPIDI_CH4R_SOURCE_SHIFT)
#define MPIDI_CH4R_TAG_SHIFT_UNPACK (sizeof(int)*8 - MPIDI_CH4R_TAG_SHIFT)

#define MPIDI_CH4I_MAP_NOT_FOUND      ((void*)(-1UL))

#define MAX_NETMOD_CONTEXTS 8
#define MAX_PROGRESS_HOOKS 4

#define MPIDI_CH4I_BUF_POOL_NUM (1024)
#define MPIDI_CH4I_BUF_POOL_SZ (256)

typedef int (*progress_func_ptr_t) (int *made_progress);
typedef struct progress_hook_slot {
    progress_func_ptr_t func_ptr;
    int active;
} progress_hook_slot_t;

typedef enum {
    MPIDI_CH4R_SEND = 0, /* Eager send */

    MPIDI_CH4R_SEND_LONG_REQ, /* Rendezvous send RTS (request to send) */
    MPIDI_CH4R_SEND_LONG_ACK, /* Rendezvous send CTS (clear to send) */
    MPIDI_CH4R_SEND_LONG_LMT, /* Rendezvous send LMT */

    MPIDI_CH4R_SSEND_REQ,
    MPIDI_CH4R_SSEND_ACK,

    MPIDI_CH4R_WIN_CTRL,

    MPIDI_CH4R_PUT_REQ,
    MPIDI_CH4R_PUT_ACK,
    MPIDI_CH4R_PUT_IOV_REQ,
    MPIDI_CH4R_PUT_DAT_REQ,
    MPIDI_CH4R_PUT_IOV_ACK,

    MPIDI_CH4R_GET_REQ,
    MPIDI_CH4R_GET_ACK,

    MPIDI_CH4R_ACC_REQ,
    MPIDI_CH4R_ACC_ACK,
    MPIDI_CH4R_ACC_IOV_REQ,
    MPIDI_CH4R_ACC_DAT_REQ,
    MPIDI_CH4R_ACC_IOV_ACK,
    MPIDI_CH4R_GET_ACC_ACK,

    MPIDI_CH4R_CSWAP_REQ,
    MPIDI_CH4R_CSWAP_ACK,
    MPIDI_CH4R_FETCH_OP
} MPIDI_CH4R_TYPE;

typedef enum {
    MPIDI_CH4R_WIN_COMPLETE,
    MPIDI_CH4R_WIN_POST,
    MPIDI_CH4R_WIN_LOCK,
    MPIDI_CH4R_WIN_LOCK_ACK,
    MPIDI_CH4R_WIN_UNLOCK,
    MPIDI_CH4R_WIN_UNLOCK_ACK,
    MPIDI_CH4R_WIN_LOCKALL,
    MPIDI_CH4R_WIN_LOCKALL_ACK,
    MPIDI_CH4R_WIN_UNLOCKALL,
    MPIDI_CH4R_WIN_UNLOCKALL_ACK
} MPIDI_CH4R_WIN_CTRL_MSG_TYPE;

enum {
    MPIDI_CH4R_EPOTYPE_NONE = 0,          /**< No epoch in affect */
    MPIDI_CH4R_EPOTYPE_LOCK = 1,          /**< MPI_Win_lock access epoch */
    MPIDI_CH4R_EPOTYPE_START = 2,         /**< MPI_Win_start access epoch */
    MPIDI_CH4R_EPOTYPE_POST = 3,          /**< MPI_Win_post exposure epoch */
    MPIDI_CH4R_EPOTYPE_FENCE = 4,         /**< MPI_Win_fence access/exposure epoch */
    MPIDI_CH4R_EPOTYPE_REFENCE = 5,       /**< MPI_Win_fence possible access/exposure epoch */
    MPIDI_CH4R_EPOTYPE_LOCK_ALL = 6       /**< MPI_Win_lock_all access epoch */
};

/* Enum for calling types between netmod and shm */
enum {
    MPIDI_CH4R_NETMOD = 0,
    MPIDI_CH4R_SHM = 1
};

typedef struct MPIDI_CH4R_Hdr_t {
    uint64_t msg_tag;
} MPIDI_CH4R_Hdr_t;

typedef struct MPIDI_CH4R_Send_long_req_msg_t {
    MPIDI_CH4R_Hdr_t hdr;
    size_t data_sz; /* Message size in bytes */
    uint64_t sreq_ptr; /* Pointer value of the request object at the sender side */
} MPIDI_CH4R_Send_long_req_msg_t;

typedef struct MPIDI_CH4R_Send_long_ack_msg_t {
    uint64_t sreq_ptr;
    uint64_t rreq_ptr;
} MPIDI_CH4R_Send_long_ack_msg_t;

typedef struct MPIDI_CH4R_Send_long_lmt_msg_t {
    uint64_t rreq_ptr;
} MPIDI_CH4R_Send_long_lmt_msg_t;

typedef struct MPIDI_CH4R_Ssend_req_msg_t {
    MPIDI_CH4R_Hdr_t hdr;
    uint64_t sreq_ptr;
} MPIDI_CH4R_Ssend_req_msg_t;

typedef struct MPIDI_CH4R_Ssend_ack_msg_t {
    uint64_t sreq_ptr;
} MPIDI_CH4R_Ssend_ack_msg_t;

typedef struct MPIDI_CH4R_win_cntrl_msg_t {
    uint64_t win_id;
    uint32_t origin_rank;
    int16_t lock_type;
    int16_t type;
} MPIDI_CH4R_win_cntrl_msg_t;

typedef struct MPIDI_CH4R_put_msg_t {
    uint64_t win_id;
    uint64_t preq_ptr;
    uint64_t addr;
    uint64_t count;
    MPI_Datatype datatype;
    int n_iov;
} MPIDI_CH4R_put_msg_t;

typedef struct MPIDI_CH4R_put_iov_ack_msg_t {
    uint64_t target_preq_ptr;
    uint64_t origin_preq_ptr;
} MPIDI_CH4R_put_iov_ack_msg_t;
typedef MPIDI_CH4R_put_iov_ack_msg_t MPIDI_CH4R_acc_iov_ack_msg_t;

typedef struct MPIDI_CH4R_put_dat_msg_t {
    uint64_t preq_ptr;
} MPIDI_CH4R_put_dat_msg_t;
typedef MPIDI_CH4R_put_dat_msg_t MPIDI_CH4R_acc_dat_msg_t;

typedef struct MPIDI_CH4R_put_ack_msg_t {
    uint64_t preq_ptr;
} MPIDI_CH4R_put_ack_msg_t;

typedef struct MPIDI_CH4R_get_req_msg_t {
    uint64_t win_id;
    uint64_t greq_ptr;
    uint64_t addr;
    uint64_t count;
    MPI_Datatype datatype;
    int n_iov;
} MPIDI_CH4R_get_req_msg_t;

typedef struct MPIDI_CH4R_get_ack_msg_t {
    uint64_t greq_ptr;
} MPIDI_CH4R_get_ack_msg_t;

typedef struct MPIDI_CH4R_cswap_req_msg_t {
    uint64_t win_id;
    uint64_t req_ptr;
    uint64_t addr;
    MPI_Datatype datatype;
} MPIDI_CH4R_cswap_req_msg_t;

typedef struct MPIDI_CH4R_cswap_ack_msg_t {
    uint64_t req_ptr;
} MPIDI_CH4R_cswap_ack_msg_t;

typedef struct MPIDI_CH4R_acc_req_msg_t {
    uint64_t win_id;
    uint64_t req_ptr;
    int origin_count;
    MPI_Datatype origin_datatype;
    int target_count;
    MPI_Datatype target_datatype;
    MPI_Op op;
    int do_get;
    uint64_t target_addr;
    uint64_t result_data_sz;
    int n_iov;
} MPIDI_CH4R_acc_req_msg_t;

typedef struct MPIDI_CH4R_acc_ack_msg_t {
    uint64_t req_ptr;
} MPIDI_CH4R_acc_ack_msg_t;

typedef struct MPIDI_CH4_Comm_req_list_t {
    MPID_Comm             *comm[2][4];
    MPIDI_CH4U_rreq_t *uelist[2][4];
} MPIDI_CH4_Comm_req_list_t;

typedef struct MPIU_buf_pool_t {
    int size;
    int num;
    void *memory_region;
    struct MPIU_buf_pool_t *next;
    struct MPIU_buf_t *head;
    pthread_mutex_t lock;
} MPIU_buf_pool_t;

typedef struct MPIU_buf_t {
    struct MPIU_buf_t *next;
    MPIU_buf_pool_t *pool;
    char data[];
} MPIU_buf_t;

typedef struct MPIDI_CH4_Global_t {
    MPID_Request *request_test;
    MPID_Comm *comm_test;
    int pname_set;
    int pname_len;
    char pname[MPI_MAX_PROCESSOR_NAME];
    int is_initialized;
    int is_ch4r_initialized;
#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO
    MPID_Node_id_t *node_map, max_node_id;
#endif
    MPIDI_CH4_Comm_req_list_t *comm_req_lists;
    OPA_int_t active_progress_hooks;
    MPID_CommOps         MPID_Comm_fns_store;
    progress_hook_slot_t progress_hooks[MAX_PROGRESS_HOOKS];
    MPID_Thread_mutex_t  m[2];
    MPID_Win *win_hash;
    int jobid;
#ifndef MPIDI_CH4R_USE_PER_COMM_QUEUE
    MPIDI_CH4U_rreq_t *posted_list;
    MPIDI_CH4U_rreq_t *unexp_list;
#endif
    MPIDI_CH4U_req_ext_t *cmpl_list;
    OPA_int_t exp_seq_no;
    OPA_int_t nxt_seq_no;
    void *netmod_context[8];
    MPIU_buf_pool_t *buf_pool;
} MPIDI_CH4_Global_t;
extern MPIDI_CH4_Global_t MPIDI_CH4_Global;
extern MPIU_Object_alloc_t MPIDI_Request_mem;
extern MPL_dbg_class MPIDI_CH4_DBG_GENERAL;
#define MPIDI_CH4I_THREAD_PROGRESS_MUTEX  MPIDI_CH4_Global.m[0]
#define MPIDI_CH4I_THREAD_PROGRESS_HOOK_MUTEX  MPIDI_CH4_Global.m[1]

#endif /* CH4_TYPES_H_INCLUDED */
