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
#include "mpid_thread_fallback.h"
#include "mpid_timers_fallback.h"

#define MPID_PROGRESS_STATE_DECL
#define HAVE_GPID_ROUTINES

#define CH4_COMPILE_TIME_ASSERT(expr_)                                  \
  do { switch(0) { case 0: case (expr_): default: break; } } while (0)

typedef enum {
    MPIDI_PTYPE_RECV,
    MPIDI_PTYPE_SEND,
    MPIDI_PTYPE_BSEND,
    MPIDI_PTYPE_SSEND,
} MPIDI_ptype;

#define MPIDI_CH4U_REQ_BUSY 		(0x1)
#define MPIDI_CH4U_REQ_PEER_SSEND 	(0x1 << 1)
#define MPIDI_CH4U_REQ_UNEXPECTED 	(0x1 << 2)
#define MPIDI_CH4U_REQ_UNEXP_DQUED 	(0x1 << 3)
#define MPIDI_CH4U_REQ_UNEXP_CLAIMED 	(0x1 << 4)
#define MPIDI_CH4U_REQ_RCV_NON_CONTIG 	(0x1 << 5)

typedef struct MPIDI_CH4U_Devreq_t {
    void *buffer;
    uint64_t count;
    uint64_t tag;
    uint64_t ignore;
    MPI_Datatype datatype;
    struct iovec *iov;
    struct MPIDI_CH4U_Devreq_t *prev, *next;

    uint64_t peer_req_ptr;
    void *reply_token;
    uint64_t status;

    /* persistent send fields */
    MPIDI_ptype p_type;

    /* mrecv fields */
    void *mrcv_buffer;
    uint64_t mrcv_count;
    MPI_Datatype mrcv_datatype;

    char netmod_am[];
} MPIDI_CH4U_Devreq_t;


typedef union {
    /* The first fields are used by the CH4U apis */
    MPIDI_CH4U_Devreq_t ch4u;

    /* Used by the netmod direct apis */
    /* Change back to 112/8 */
    uint64_t netmod[512 / 8];
} MPIDI_Devreq_t;
#define MPIDI_REQUEST_HDR_SIZE              offsetof(struct MPID_Request, dev.netmod)
#define MPIDI_REQUEST_CH4U_HDR_SIZE         offsetof(struct MPID_Request, dev.netmod_am)
#define MPIU_CH4U_REQUEST(req,field)        (((req)->dev.ch4u).field)
#define MPIU_CH4U_REQUEST_AM_NETMOD(req)    (((req)->dev.ch4u).netmod_am)
#define MPIU_CH4_NETMOD_DIRECT_REQUEST(req) ((req)->dev.netmod)

typedef struct MPIDI_CH4U_win_basic_info_t {
    uint64_t base;
    uint64_t size;
    uint64_t disp_unit;
    uint64_t win_ptr;
} MPIDI_CH4U_win_basic_info_t;

typedef struct MPIDI_CH4U_win_info_args_t {
    int no_locks;               /* valid flavor = all */
    int accumulate_ordering;
    int accumulate_ops;
    int same_size;              /* valid flavor = allocate */
    int alloc_shared_noncontig; /* valid flavor = allocate shared */
    int alloc_shm;              /* valid flavor = allocate */
} MPIDI_CH4U_win_info_args_t;

typedef struct MPIDI_CH4U_win_t {
    MPIDI_CH4U_win_basic_info_t *basic_info_table;
    MPIDI_CH4U_win_info_args_t info_args;
} MPIDI_CH4U_win_t;

typedef struct {
    MPIDI_CH4U_win_t ch4u;
    uint64_t pad[192 / 8];
} MPIDI_Devwin_t;
#define MPIU_CH4U_WIN(win,field)        (((win)->dev.ch4u).field)

typedef struct MPIDI_CH4U_Devcomm_t {
    MPIDI_CH4U_Devreq_t *posted_list;
    MPIDI_CH4U_Devreq_t *unexp_list;
} MPIDI_CH4U_Devcomm_t;

typedef struct {
    MPIDI_CH4U_Devcomm_t ch4u;
    uint64_t pad[256 / 8];
} MPIDI_Devcomm_t;
#define MPIU_CH4U_COMM(comm,field) ((comm)->dev.ch4u).field

typedef struct {
    uint32_t pad[4 / 4];
} MPIDI_Devdt_t;

typedef struct {
    uint64_t pad[32 / 8];
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

#include "mpid_sched_pre.h"

#endif /* MPIDPRE_H_INCLUDED */
