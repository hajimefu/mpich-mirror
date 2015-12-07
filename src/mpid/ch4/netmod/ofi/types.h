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
#include "mpidch4u.h"

EXTERN_C_BEGIN
#define __SHORT_FILE__                          \
  (strrchr(__FILE__,'/')                        \
   ? strrchr(__FILE__,'/')+1                    \
   : __FILE__                                   \
)
#ifndef MIN
#define MIN(x,y) (((x)<(y))?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y))?(x):(y))
#endif
#define MPIDI_MAP_NOT_FOUND      ((void*)(-1UL))
#define MPIDI_FI_MAJOR_VERSION 1
#define MPIDI_FI_MINOR_VERSION 0
#define MPIDI_MAX_SHORT_SEND_SZ		(16 * 1024)
#define MPIDI_MIN_MSG_SZ		      MPIDI_MAX_SHORT_SEND_SZ
#define MPIDI_NUM_AM_BUFFERS		  (8)
#define MPIDI_AM_BUFF_SZ		      (1 * 1024 * 1024)
#define MPIDI_MAX_AM_HANDLERS 		(16)
#define MPIDI_CACHELINE_SIZE        (64)
#define MPIDI_IOV_MAX               1
#define MPIDI_BUF_POOL_SZ  (1024)
#define MPIDI_BUF_POOL_NUM (1024)

/* Macros and inlines */
/* match/ignore bit manipulation
 *
 * 0123 4567 01234567 0123 4567 01234567 0123 4567 01234567 01234567 01234567
 *     |                  |                  |
 * ^   |    context id    |       source     |       message tag
 * |   |                  |                  |
 * +---- protocol
 */
#define MPID_PROTOCOL_MASK (0x9000000000000000ULL)
#define MPID_CONTEXT_MASK  (0x0FFFF00000000000ULL)
#define MPID_SOURCE_MASK   (0x00000FFFF0000000ULL)
#define MPID_TAG_MASK      (0x000000000FFFFFFFULL)
#define MPID_SYNC_SEND     (0x1000000000000000ULL)
#define MPID_SYNC_SEND_ACK (0x2000000000000000ULL)
#define MPID_DYNPROC_SEND  (0x4000000000000000ULL)
#define MPID_TAG_SHIFT     (28)
#define MPID_SOURCE_SHIFT  (16)

/* RMA Key Space division
 *    |                  |                  |                    |
 *    ...     Context ID |   Huge RMA       |  Window Instance   |
 *    |                  |                  |                    |
 */
/* 64-bit key space                         */
/* 2M  window instances per comm           */
/* 2M  outstanding huge RMAS per comm      */
/* 4M  communicators                       */
#define MPIDI_MAX_WINDOWS_BITS_64  (21)
#define MPIDI_MAX_HUGE_RMA_BITS_64 (21)
#define MPIDI_MAX_HUGE_RMAS_64     (1<<(MPIDI_MAX_HUGE_RMA_BITS_64))
#define MPIDI_MAX_WINDOWS_64       (1<<(MPIDI_MAX_WINDOWS_BITS_64))
#define MPIDI_HUGE_RMA_SHIFT_64    (MPIDI_MAX_WINDOWS_BITS_64)
#define MPIDI_CONTEXT_SHIFT_64     (MPIDI_MAX_WINDOWS_BITS_64+MPIDI_MAX_HUGE_RMA_BITS_64)

/* 32-bit key space                         */
/* 4096 window instances per comm           */
/* 256  outstanding huge RMAS per comm      */
/* 4096 communicators                       */
#define MPIDI_MAX_WINDOWS_BITS_32  (12)
#define MPIDI_MAX_HUGE_RMA_BITS_32 (8)
#define MPIDI_MAX_HUGE_RMAS_32     (1<<(MPIDI_MAX_HUGE_RMA_BITS_32))
#define MPIDI_MAX_WINDOWS_32       (1<<(MPIDI_MAX_WINDOWS_BITS_32))
#define MPIDI_HUGE_RMA_SHIFT_32    (MPIDI_MAX_WINDOWS_BITS_32)
#define MPIDI_CONTEXT_SHIFT_32     (MPIDI_MAX_WINDOWS_BITS_32+MPIDI_MAX_HUGE_RMA_BITS_32)

/* 16-bit key space                         */
/* 64 window instances per comm             */
/* 16 outstanding huge RMAS per comm        */
/* 64 communicators                          */
#define MPIDI_MAX_WINDOWS_BITS_16  (6)
#define MPIDI_MAX_HUGE_RMA_BITS_16 (4)
#define MPIDI_MAX_HUGE_RMAS_16     (1<<(MPIDI_MAX_HUGE_RMA_BITS_16))
#define MPIDI_MAX_WINDOWS_16       (1<<(MPIDI_MAX_WINDOWS_BITS_16))
#define MPIDI_HUGE_RMA_SHIFT_16    (MPIDI_MAX_WINDOWS_BITS_16)
#define MPIDI_CONTEXT_SHIFT_16     (MPIDI_MAX_WINDOWS_BITS_16+MPIDI_MAX_HUGE_RMA_BITS_16)


/* Typedefs */
typedef struct iovec iovec_t;
typedef int (*event_event_fn) (cq_tagged_entry_t * wc, MPID_Request *);
typedef int (*control_event_fn) (void *buf);

typedef enum {
    MPIDI_ACCU_ORDER_RAR = 1,
    MPIDI_ACCU_ORDER_RAW = 2,
    MPIDI_ACCU_ORDER_WAR = 4,
    MPIDI_ACCU_ORDER_WAW = 8
} MPIDI_Win_info_accumulate_ordering;

typedef enum {
    MPIDI_ACCU_SAME_OP,
    MPIDI_ACCU_SAME_OP_NO_OP
} MPIDI_Win_info_accumulate_ops;

enum {
    MPID_EPOTYPE_NONE = 0,          /**< No epoch in affect */
    MPID_EPOTYPE_LOCK = 1,          /**< MPI_Win_lock access epoch */
    MPID_EPOTYPE_START = 2,         /**< MPI_Win_start access epoch */
    MPID_EPOTYPE_POST = 3,          /**< MPI_Win_post exposure epoch */
    MPID_EPOTYPE_FENCE = 4,         /**< MPI_Win_fence access/exposure epoch */
    MPID_EPOTYPE_REFENCE = 5,       /**< MPI_Win_fence possible access/exposure epoch */
    MPID_EPOTYPE_LOCK_ALL = 6,      /**< MPI_Win_lock_all access epoch */
};

enum {
    MPIDI_CTRL_ASSERT,    /**< Lock acknowledge      */
    MPIDI_CTRL_LOCKACK,   /**< Lock acknowledge      */
    MPIDI_CTRL_LOCKALLACK,/**< Lock all acknowledge  */
    MPIDI_CTRL_LOCKREQ,   /**< Lock window           */
    MPIDI_CTRL_LOCKALLREQ,/**< Lock all window       */
    MPIDI_CTRL_UNLOCK,    /**< Unlock window         */
    MPIDI_CTRL_UNLOCKACK, /**< Unlock window         */
    MPIDI_CTRL_UNLOCKALL, /**< Unlock window         */
    MPIDI_CTRL_UNLOCKALLACK,
    /**< Unlock window         */
    MPIDI_CTRL_COMPLETE,  /**< End a START epoch     */
    MPIDI_CTRL_POST,      /**< Begin POST epoch      */
    MPIDI_CTRL_HUGE,      /**< Huge message          */
    MPIDI_CTRL_HUGEACK,   /**< Huge message ack      */
    MPIDI_CTRL_HUGE_CLEANUP,
    /**< Huge message cleanup  */
};

enum {
    MPIDI_EVENT_ABORT,
    MPIDI_EVENT_PEEK,
    MPIDI_EVENT_RECV,
    MPIDI_EVENT_RECV_HUGE,
    MPIDI_EVENT_SEND,
    MPIDI_EVENT_SEND_HUGE,
    MPIDI_EVENT_SSEND_ACK,
    MPIDI_EVENT_GET_HUGE,
    MPIDI_EVENT_CONTROL,
    MPIDI_EVENT_CHUNK_DONE,
    MPIDI_EVENT_RMA_DONE,
    MPIDI_EVENT_DYNPROC_DONE,
    MPIDI_EVENT_ACCEPT_PROBE
};

enum {
    MPIDI_REQUEST_LOCK,
    MPIDI_REQUEST_LOCKALL,
};

enum {
    MPIDI_PEEK_START,
    MPIDI_PEEK_NOT_FOUND,
    MPIDI_PEEK_FOUND
};


/* Physical address table data */
typedef struct {
    fi_addr_t dest;
    /**< A single physical address */
} MPID_Addr_entry_t;
typedef struct {
    int size;
    MPID_Addr_entry_t table[0];/**< Array of physical addresses */
} MPIDI_Addr_table_t;

typedef struct {
    char pad[MPIDI_REQUEST_HDR_SIZE];
    context_t context;          /* fixed field, do not move */
    int       event_id;
} MPIDI_Ctrl_req;

typedef struct {
    char pad[MPIDI_REQUEST_HDR_SIZE];
    context_t context;          /* fixed field, do not move */
    int event_id; /* fixed field, do not move */
    MPID_Request *signal_req;
} MPIDI_Ssendack_request;

typedef struct {
    char pad[MPIDI_REQUEST_HDR_SIZE];
    context_t context;          /* fixed field, do not move */
    int event_id; /* fixed field, do not move */
    int done;
    uint32_t tag;
    uint32_t source;
    uint64_t msglen;
} MPIDI_Dynproc_req;

typedef struct atomic_valid {
    uint8_t op;
    uint8_t dt;
    unsigned atomic_valid:2;
    unsigned fetch_atomic_valid:2;
    unsigned compare_atomic_valid:2;
    unsigned dtsize:10;
    uint64_t max_atomic_count;
    uint64_t max_compare_atomic_count;
    uint64_t max_fetch_atomic_count;
} atomic_valid_t;
#define DT_SIZES 62
#define OP_SIZES 15

#define MPIDI_API_TAG 0
#define MPIDI_API_RMA 1
#define MPIDI_API_MSG 2
#define MPIDI_API_CTR 3

#define MPIDI_THREAD_UTIL_MUTEX     MPIDI_Global.mutexes[0].m
#define MPIDI_THREAD_PROGRESS_MUTEX MPIDI_Global.mutexes[1].m
#define MPIDI_THREAD_FI_MUTEX       MPIDI_Global.mutexes[2].m
#define MPIDI_THREAD_SPAWN_MUTEX    MPIDI_Global.mutexes[3].m

typedef struct {
    fid_ep_t tx_tag;
    fid_ep_t rx_tag;

    fid_ep_t tx_rma;
    fid_ep_t rx_rma;

    fid_ep_t tx_msg;
    fid_ep_t rx_msg;

    fid_ep_t tx_ctr;
    fid_ep_t rx_ctr;

    int ctx_offset;
} MPIDI_Context_t;

typedef union MPIDI_cacheline_mutex_t{
    MPID_Thread_mutex_t m;
    char cacheline[MPIDI_CACHELINE_SIZE];
}MPIDI_cacheline_mutex_t __attribute__ ((aligned (MPIDI_CACHELINE_SIZE)));

/* Global state data */
#define MPIDI_KVSAPPSTRLEN 1024
typedef struct {
    int jobid;
    char addrname[FI_NAME_MAX];
    size_t addrnamelen;
    fid_domain_t domain;
    fid_fabric_t fabric;
    fid_base_ep_t ep;
#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
    MPIDI_Context_t ctx[MPIDI_MAX_ENDPOINTS];
#endif
    fid_cq_t p2p_cq;
    fid_cq_t am_cq;
    fid_cntr_t rma_ctr;
    fid_av_t av;
    iovec_t *iov;
    MPIDI_cacheline_mutex_t mutexes[4];
    msg_t *msg;
    MPIDI_Ctrl_req *control_req;
    uint64_t cntr;
    uint64_t max_buffered_send;
    uint64_t max_buffered_write;
    uint64_t max_send;
    uint64_t max_write;
    uint64_t max_short_send;
    uint64_t max_mr_key_size;
    int      max_windows_bits;
    int      max_huge_rma_bits;
    int      max_huge_rmas;
    int      huge_rma_shift;
    int      context_shift;
    size_t iov_limit;
    int cur_ctrlblock;
    int num_ctrlblock;
    int control_init;
    control_event_fn control_fn[16];
    MPID_Node_id_t *node_map;
    MPID_Node_id_t max_node_id;
    void *win_map;
    void *comm_map;
    atomic_valid_t win_op_table[DT_SIZES][OP_SIZES];
    MPID_CommOps MPID_Comm_fns_store;
    struct iovec am_iov[MPIDI_NUM_AM_BUFFERS];
    struct fi_msg am_msg[MPIDI_NUM_AM_BUFFERS];
    void *am_bufs[MPIDI_NUM_AM_BUFFERS];
    MPIDI_netmod_am_target_handler_fn am_handlers[MPIDI_MAX_AM_HANDLERS];
    MPIDI_netmod_am_origin_handler_fn send_cmpl_handlers[MPIDI_MAX_AM_HANDLERS];
    int coll_progress;
    int pname_set;
    int pname_len;
    char kvsname[MPIDI_KVSAPPSTRLEN];
    char pname[MPI_MAX_PROCESSOR_NAME];
    int port_name_tag_mask[MPIR_MAX_CONTEXT_MASK];
    MPIU_buf_pool_t *buf_pool;
} MPIDI_Global_t;

typedef struct {
    char addr[30];
    MPID_Node_id_t node;
} MPIDI_OFIGpid_t;
#define GPID_OFI(req) ((MPIDI_OFIGpid_t*)(req)->dev.pad)


typedef struct {
    uint32_t index;
} MPIDI_OFIdt_t;
#define DT_OFI(comm) ((MPIDI_OFIdt_t*)(comm)->dev.pad)

struct MPIDI_Win_lock {
    struct MPIDI_Win_lock *next;
    unsigned rank;
    uint16_t mtype;
    uint16_t type;
};

struct MPIDI_Win_queue {
    struct MPIDI_Win_lock *head;
    struct MPIDI_Win_lock *tail;
};

typedef struct MPIDI_WinLock_info {
    unsigned peer;
    int lock_type;
    struct MPID_Win *win;
    volatile unsigned done;
} MPIDI_WinLock_info;

/* These control structures have to be the same size */
typedef struct {
    int16_t  type;
    int16_t  lock_type;
    int      origin_rank;
    uint64_t win_id;
    int      dummy[8];
} MPIDI_Win_control_t;
#define MPID_MIN_CTRL_MSG_SZ (sizeof(MPIDI_Win_control_t))

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
} MPIDI_Send_control_t;

typedef struct {
    void *addr;
    void *result_addr;
    void *req;
    MPID_Win *win;
    MPI_Datatype type;
    MPI_Op op;
    int    origin_endpoint;
    size_t len;
} MPIDI_Win_MsgInfo;

typedef struct MPIDI_dummy {
    MPIU_OBJECT_HEADER;
    void *pad;
} MPIDI_dummy_t;
#define MPIDI_OBJECT_HEADER_SZ offsetof(struct MPIDI_dummy,  pad)

typedef struct MPIDI_Iovec_state {
    uintptr_t  target_base_addr;
    uintptr_t  origin_base_addr;
    uintptr_t  result_base_addr;
    size_t     target_count;
    size_t     origin_count;
    size_t     result_count;
    iovec_t   *target_iov;
    iovec_t   *origin_iov;
    iovec_t   *result_iov;
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
} MPIDI_Iovec_state_t;

typedef struct {
    MPID_Datatype  *pointer;
    MPI_Datatype    type;
    int             count;
    int             contig;
    MPI_Aint        true_lb;
    MPIDI_msg_sz_t  size;
    int             num_contig;
    DLOOP_VECTOR   *map;
    DLOOP_VECTOR    __map;
} MPIDI_Win_dt;

typedef struct MPIDI_Iovec_array {
    char                      pad[MPIDI_REQUEST_HDR_SIZE];
    context_t                 context;   /* fixed field, do not move */
    int                       event_id;  /* fixed field, do not move */
    struct MPIDI_Iovec_array *next;
    union {
        struct {
            iovec_t   *originv;
            rma_iov_t *targetv;
        }put_get;
        struct {
            ioc_t     *originv;
            rma_ioc_t *targetv;
            ioc_t     *resultv;
            ioc_t     *comparev;
        }cas;
        struct {
            ioc_t     *originv;
            rma_ioc_t *targetv;
        }accumulate;
        struct {
            ioc_t     *originv;
            rma_ioc_t *targetv;
            ioc_t     *resultv;
        }get_accumulate;
    }iov;
    char iov_store[0]; /* Flexible array, do not move */
} MPIDI_Iovec_array_t;


typedef struct MPIDI_Win_noncontig {
    MPIDI_Iovec_state_t iovs;
    MPIDI_Win_dt        origin_dt;
    MPIDI_Win_dt        target_dt;
    MPIDI_Win_dt        result_dt;
    MPIDI_Iovec_array_t buf; /* Do not move me, flexible array */
} MPIDI_Win_noncontig;

typedef struct MPIDI_Win_request {
    MPIU_OBJECT_HEADER;
    char                      pad[MPIDI_REQUEST_HDR_SIZE - MPIDI_OBJECT_HEADER_SZ];
    context_t                 context;          /* fixed field, do not move */
    int                       event_id;         /* fixed field, do not move */
    struct MPIDI_Win_request *next;
    int                       target_rank;
    MPIDI_Win_noncontig      *noncontig;
} MPIDI_Win_request;

typedef struct {
    char pad[MPIDI_REQUEST_HDR_SIZE];
    context_t     context;          /* fixed field, do not move */
    int           event_id;         /* fixed field, do not move */
    MPID_Request *parent;           /* Parent request           */
} MPIDI_Chunk_request;

typedef struct {
    char                  pad[MPIDI_REQUEST_HDR_SIZE];
    context_t             context;          /* fixed field, do not move */
    int                   event_id; /* fixed field, do not move */
    event_event_fn        done_fn;
    MPIDI_Send_control_t  remote_info;
    size_t                cur_offset;
    MPID_Comm            *comm_ptr;
    MPID_Request         *localreq;
    cq_tagged_entry_t     wc;
} MPIDI_Huge_chunk_t;

typedef struct {
    uint16_t seqno;
    void *chunk_q;
} MPIDI_Huge_recv_t;

typedef struct MPIDI_Hugecntr {
    uint16_t counter;
    uint16_t outstanding;
    fid_mr_t mr;
} MPIDI_Hugecntr;

typedef struct MPIDI_Win_info_args {
    int no_locks;
    MPIDI_Win_info_accumulate_ordering accumulate_ordering;
    MPIDI_Win_info_accumulate_ops accumulate_ops;
    int same_size;
    int alloc_shared_noncontig;
} MPIDI_Win_info_args;

typedef struct MPIDI_Win_info {
    void *base_addr;
    uint32_t disp_unit;
} __attribute__ ((packed)) MPIDI_Win_info;

typedef struct MPIDI_Win_sync_lock {
    struct {
        volatile unsigned locked;
        volatile unsigned allLocked;
    } remote;
    struct {
        struct MPIDI_Win_queue requested;
        int type;
        unsigned count;
    } local;
} MPIDI_Win_sync_lock;

typedef struct MPIDI_Win_sync_pscw {
    struct MPID_Group *group;
    volatile unsigned count;
} MPIDI_Win_sync_pscw;

typedef struct MPIDI_Win_sync_t {
    volatile int origin_epoch_type;
    volatile int target_epoch_type;
    MPIDI_Win_sync_pscw sc, pw;
    MPIDI_Win_sync_lock lock;
} MPIDI_Win_sync_t;

typedef struct {
    void                *winfo;
    MPIDI_Win_sync_t     sync;
    fid_mr_t             mr;
    uint64_t             mr_key;
    uint64_t             win_id;
    void                *mmap_addr;
    int64_t              mmap_sz;
    MPI_Aint            *sizes;
    MPIDI_Win_request   *syncQ;
    void                *msgQ;
    int                  count;
    MPIDI_Win_info_args  info_args;
} MPIDI_OFIWin_t;
#define WIN_OFI(win) ((MPIDI_OFIWin_t*)(win)->dev.pad)

#define MPID_LKEY_START 16384

extern MPIDI_Addr_table_t *MPIDI_Addr_table;
extern MPIDI_Global_t MPIDI_Global;
extern MPIU_Object_alloc_t MPIDI_Request_mem;
extern MPID_Request MPIDI_Request_direct[];

/* Utility functions */
extern int   MPIDI_OFI_VCRT_Create(int size, struct MPIDI_VCRT **vcrt_ptr);
extern int   MPIDI_OFI_VCRT_Release(struct MPIDI_VCRT *vcrt);
extern void  MPIDI_OFI_Map_create(void **map);
extern void  MPIDI_OFI_Map_destroy(void *map);
extern void  MPIDI_OFI_Map_set(void *_map, uint64_t id, void *val);
extern void  MPIDI_OFI_Map_erase(void *_map, uint64_t id);
extern void *MPIDI_OFI_Map_lookup(void *_map, uint64_t id);
extern int   MPIDI_OFI_control_dispatch(void *buf);
extern void  MPIDI_OFI_Index_datatypes();
extern void  MPIDI_OFI_Index_allocator_create(void **_indexmap, int start);
extern int   MPIDI_OFI_Index_allocator_alloc(void *_indexmap);
extern void  MPIDI_OFI_Index_allocator_free(void *_indexmap, int index);
extern void  MPIDI_OFI_Index_allocator_destroy(void *_indexmap);

extern int   MPIR_Datatype_init_names(void);

EXTERN_C_END
#endif /* NETMOD_OFI_IMPL_H_INCLUDED */
