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

#ifndef MPIDPRE_H_INCLUDED
#define MPIDPRE_H_INCLUDED

#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif
typedef size_t MPIDI_msg_sz_t;

#include "mpid_dataloop.h"
#include "mpid_thread.h"
#include "mpid_sched.h"
#include "mpid_timers_fallback.h"
#include "netmodpre.h"
#include "mpl_uthash.h"

#define MPID_PROGRESS_STATE_DECL
#define HAVE_GPID_ROUTINES

#define __ALWAYS_INLINE__ __attribute__((always_inline)) static inline

#define CH4_COMPILE_TIME_ASSERT(expr_)                                  \
  do { switch(0) { case 0: case (expr_): default: break; } } while (0)

typedef enum {
    MPIDI_PTYPE_RECV,
    MPIDI_PTYPE_SEND,
    MPIDI_PTYPE_BSEND,
    MPIDI_PTYPE_SSEND,
} MPIDI_ptype;

enum {
    MPIDI_CH4_REQTYPE_MPI,
    MPIDI_CH4_REQTYPE_AM,
    MPIDI_CH4_REQTYPE_NATIVE,
    MPIDI_CH4_REQTYPE_AM_SHM,
    MPIDI_CH4_REQTYPE_NATIVE_SHM
};

#define MPIDI_CH4R_REQ_BUSY 		  (0x1)
#define MPIDI_CH4R_REQ_PEER_SSEND 	  (0x1 << 1)
#define MPIDI_CH4R_REQ_UNEXPECTED 	  (0x1 << 2)
#define MPIDI_CH4R_REQ_UNEXP_DQUED 	  (0x1 << 3)
#define MPIDI_CH4R_REQ_UNEXP_CLAIMED  (0x1 << 4)
#define MPIDI_CH4R_REQ_RCV_NON_CONTIG (0x1 << 5)
#define MPIDI_CH4R_REQ_MATCHED (0x1 << 6)

#define MPIDI_PARENT_PORT_KVSKEY "PARENT_ROOT_PORT_NAME"
#define MPIDI_MAX_KVS_VALUE_LEN  4096

typedef struct MPIDI_CH4R_Dev_sreq_t {
    /* persistent send fields */
} MPIDI_CH4R_Dev_sreq_t;

typedef struct MPIDI_CH4R_Dev_rreq_t {
    /* mrecv fields */
    void         *mrcv_buffer;
    uint64_t      mrcv_count;
    MPI_Datatype  mrcv_datatype;

    uint64_t      ignore;
    uint64_t      reply_token;
    uint64_t      peer_req_ptr;
    uint64_t      match_req;
    uint64_t      request;

    struct MPIDI_CH4R_Dev_rreq_t *prev, *next;
} MPIDI_CH4R_Dev_rreq_t;

typedef struct MPIDI_CH4R_Dev_put_req_t {
    uint64_t win_ptr;
    uint64_t preq_ptr;
    uint64_t reply_token;
    void *dt_iov;
    void *origin_addr;
    int origin_count;
    MPI_Datatype origin_datatype;
    int n_iov;
} MPIDI_CH4R_Dev_put_req_t;

typedef struct MPIDI_CH4R_Dev_get_req_t {
    uint64_t win_ptr;
    uint64_t greq_ptr;
    uint64_t addr;
    MPI_Datatype datatype;
    int count;
    int n_iov;
    uint64_t reply_token;
    void *dt_iov;
} MPIDI_CH4R_Dev_get_req_t;

typedef struct MPIDI_CH4R_Dev_cswap_req_t {
    uint64_t win_ptr;
    uint64_t creq_ptr;
    uint64_t reply_token;
    uint64_t addr;
    MPI_Datatype datatype;
    void *data;
    void *result_addr;
} MPIDI_CH4R_Dev_cswap_req_t;

typedef struct MPIDI_CH4R_Dev_acc_req_t {
    uint64_t win_ptr;
    uint64_t req_ptr;
    uint64_t reply_token;
    MPI_Datatype origin_datatype;
    MPI_Datatype target_datatype;
    int origin_count;
    int target_count;
    int n_iov;
    void *target_addr;
    void *dt_iov;
    void *data;
    size_t data_sz;
    MPI_Op op;
    void *result_addr;
    int result_count;
    int do_get;
    void *origin_addr;
    MPI_Datatype result_datatype;
} MPIDI_CH4R_Dev_acc_req_t;

typedef struct MPIDI_CH4R_req_t {
    union {
        MPIDI_CH4R_Dev_sreq_t sreq;
        MPIDI_CH4R_Dev_rreq_t rreq;
        MPIDI_CH4R_Dev_put_req_t preq;
        MPIDI_CH4R_Dev_get_req_t greq;
        MPIDI_CH4R_Dev_cswap_req_t creq;
        MPIDI_CH4R_Dev_acc_req_t areq;
    };

    struct iovec *iov;
    void         *cmpl_handler_fn;
    uint64_t      seq_no;
    uint64_t      request;
    uint64_t      status;
    struct MPIDI_CH4R_req_t *next, *prev;

} MPIDI_CH4R_req_t;

typedef struct MPIDI_CH4R_Devreq_t {
    union {
        MPIDI_CH4_NETMOD_REQUEST_AM_DECL
    }netmod_am;
    MPIDI_CH4R_req_t *req;
    MPIDI_ptype       p_type;
    void             *buffer;
    uint64_t          count;
    uint64_t          tag;
    MPI_Datatype      datatype;
    struct MPID_Comm *util_comm;
} MPIDI_CH4R_Devreq_t;

typedef struct {
#ifdef MPIDI_CH4_EXCLUSIVE_SHM
    int16_t is_local;
    int16_t reqtype;
#else
    int reqtype;
#endif

    /* Anysource handling. Netmod and shm specific requests are cross
     * referenced. This must be present all of the time to avoid lots of extra
     * ifdefs in the code. */
#ifdef MPIDI_BUILD_CH4_SHM
    struct MPID_Request *anysource_partner_request;
#endif

    union {
        /* The first fields are used by the CH4U apis */
        MPIDI_CH4R_Devreq_t ch4r;

        /* Used by the netmod direct apis */
        union {
            MPIDI_CH4_NETMOD_REQUEST_DECL
        }netmod;

        union {
            MPIDI_CH4_SHM_REQUEST_DECL
        }shm;
    }ch4;
} MPIDI_Devreq_t;
#define MPIDI_REQUEST_HDR_SIZE              offsetof(struct MPID_Request, dev.ch4.netmod)
#define MPIDI_REQUEST_CH4U_HDR_SIZE         offsetof(struct MPID_Request, dev.ch4.netmod_am)
#define MPIDI_CH4I_REQUEST(req,field)       (((req)->dev).field)
#define MPIDI_CH4R_REQUEST(req,field)       (((req)->dev.ch4.ch4r).field)

#ifdef MPIDI_BUILD_CH4_SHM
#define MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req)  (((req)->dev).anysource_partner_request)
#else
#define MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req)  NULL
#endif

typedef struct MPIDI_CH4R_win_info_t {
    uint64_t base_addr;
    uint32_t disp_unit;
} __attribute__ ((packed)) MPIDI_CH4R_win_info_t;

#define MPIDI_CH4I_ACCU_ORDER_RAR (1)
#define MPIDI_CH4I_ACCU_ORDER_RAW (1 << 1)
#define MPIDI_CH4I_ACCU_ORDER_WAR (1 << 2)
#define MPIDI_CH4I_ACCU_ORDER_WAW (1 << 3)

typedef enum {
    MPIDI_CH4I_ACCU_SAME_OP,
    MPIDI_CH4I_ACCU_SAME_OP_NO_OP
} MPIDI_CH4R_win_info_accumulate_ops;

typedef struct MPIDI_CH4R_win_info_args_t {
    int no_locks;
    int same_size;
    int accumulate_ordering;
    int alloc_shared_noncontig;
    MPIDI_CH4R_win_info_accumulate_ops accumulate_ops;
} MPIDI_CH4R_win_info_args_t;

struct MPIDI_CH4R_win_lock {
    struct MPIDI_CH4R_win_lock *next;
    unsigned rank;
    uint16_t mtype;
    uint16_t type;
};

struct MPIDI_CH4R_win_queue {
    struct MPIDI_CH4R_win_lock *head;
    struct MPIDI_CH4R_win_lock *tail;
};

typedef struct MPIDI_CH4R_winLock_info {
    unsigned peer;
    int lock_type;
    struct MPID_Win *win;
    volatile unsigned done;
} MPIDI_CH4R_winLock_info;

typedef struct MPIDI_CH4R_win_sync_lock {
    struct {
        volatile unsigned locked;
        volatile unsigned allLocked;
    } remote;
    struct {
        struct MPIDI_CH4R_win_queue requested;
        int type;
        unsigned count;
    } local;
} MPIDI_CH4R_win_sync_lock;

typedef struct MPIDI_CH4R_win_sync_pscw {
    struct MPID_Group *group;
    volatile unsigned count;
} MPIDI_CH4R_win_sync_pscw;

typedef struct MPIDI_CH4R_win_sync_t {
    volatile int origin_epoch_type;
    volatile int target_epoch_type;
    MPIDI_CH4R_win_sync_pscw sc, pw;
    MPIDI_CH4R_win_sync_lock lock;
} MPIDI_CH4R_win_sync_t;

typedef struct MPIDI_CH4R_win_t {
    uint64_t win_id;
    void *mmap_addr;
    int64_t mmap_sz;
    OPA_int_t outstanding_ops;
    MPI_Aint *sizes;
    void *msgQ;
    void *syncQ; /* todo */
    int count;
    MPIDI_CH4R_win_sync_t sync;
    MPIDI_CH4R_win_info_t *info_table;
    MPIDI_CH4R_win_info_args_t info_args;
    MPL_UT_hash_handle hash_handle;
} MPIDI_CH4R_win_t;

typedef struct {
    MPIDI_CH4R_win_t ch4r;
    uint64_t pad[192 / 8];
} MPIDI_Devwin_t;
#define MPIDI_CH4R_WIN(win,field)        (((win)->dev.ch4r).field)
#define MPIDI_CH4R_WINFO(win,rank) (MPIDI_CH4R_win_info_t*) &(MPIDI_CH4R_WIN(win, info_table)[rank])
#define MPIDI_CH4R_WINFO_DISP_UNIT(w,rank)                              \
    ({                                                                  \
        uint32_t _v;                                                    \
        if(MPIDI_CH4R_WIN(w, info_table)) {                             \
            _v = ((MPIDI_CH4R_win_info_t *) MPIDI_CH4R_WINFO(w, rank))->disp_unit; \
        }                                                               \
        else {                                                          \
            _v = w->disp_unit;                                          \
        }                                                               \
        _v;                                                             \
    })


typedef struct {
    unsigned is_local : 1;
    unsigned index    : 31;
}MPIDI_CH4R_locality_t;

typedef struct MPIDI_CH4R_Devcomm_t {
    MPIDI_CH4R_Dev_rreq_t *posted_list;
    MPIDI_CH4R_Dev_rreq_t *unexp_list;
    uint32_t   window_instance;
#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO
    MPIDI_CH4R_locality_t *locality;
#endif
} MPIDI_CH4R_Devcomm_t;

typedef struct MPIDI_Devcomm_t {
    struct {
        /* The first fields are used by the CH4U apis */
        MPIDI_CH4R_Devcomm_t ch4r;

        /* Used by the netmod direct apis */
        union {
            MPIDI_CH4_NETMOD_COMM_DECL
        }netmod;

        union {
            MPIDI_CH4_SHM_COMM_DECL
        }shm;
    }ch4;
} MPIDI_Devcomm_t;
#define MPIDI_CH4R_COMM(comm,field) ((comm)->dev.ch4.ch4r).field

typedef struct {
    uint32_t pad[4 / 4];
} MPIDI_Devdt_t;

typedef struct {
    uint64_t pad[64 / 8];
} MPIDI_Devgpid_t;

#define MPID_DEV_REQUEST_DECL    MPIDI_Devreq_t  dev;
#define MPID_DEV_WIN_DECL        MPIDI_Devwin_t  dev;
#define MPID_DEV_COMM_DECL       MPIDI_Devcomm_t dev;
#define MPID_DEV_DATATYPE_DECL   MPIDI_Devdt_t   dev;
#define MPID_DEV_GPID_DECL       MPIDI_Devgpid_t dev;

#define MPID_Progress_register_hook(fn_, id_) MPID_Progress_register(fn_, id_)
#define MPID_Progress_deregister_hook(id_) MPID_Progress_deregister(id_)
#define MPID_Progress_activate_hook(id_) MPID_Progress_activate(id_)
#define MPID_Progress_deactivate_hook(id_) MPID_Progress_deactivate(id_)

#define HAVE_DEV_COMM_HOOK
#define MPID_Dev_comm_create_hook(a)  (MPID_Comm_create(a))
#define MPID_Dev_comm_destroy_hook(a) (MPID_Comm_destroy(a))

#define MPID_USE_NODE_IDS
typedef uint16_t MPID_Node_id_t;

#include "mpidu_pre.h"

#endif /* MPIDPRE_H_INCLUDED */
