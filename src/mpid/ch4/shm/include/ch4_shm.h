/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/* ch4 shm functions */
#ifndef SHM_PROTOTYPES_H_INCLUDED
#define SHM_PROTOTYPES_H_INCLUDED

#include <mpidimpl.h>

#define MPIDI_MAX_SHM_STRING_LEN 64

typedef int (*MPIDI_shm_am_completion_handler_fn) (MPID_Request * req);
typedef int (*MPIDI_shm_am_origin_handler_fn) (MPID_Request * req);

/* Callback function setup by handler register function */
/* for short cases, output arguments are NULL */
typedef int (*MPIDI_shm_am_target_handler_fn)
 (void *am_hdr, size_t am_hdr_sz, void *reply_token,    /* contains information about reply operation */
  void **data,                  /* CH4 manages this buffer - shm only fills with data */
  MPI_Datatype * datatype, MPI_Count * count, int *noncontig,   /* if TRUE: data/data_sz are actually iovec/count */
  MPIDI_shm_am_completion_handler_fn * cmpl_handler_fn, /* completion handler */
  MPID_Request ** req);         /* if allocated, need pointer to completion function */

#ifndef MPIDI_CH4_ARRAY_TYPEDEFS
#define MPIDI_CH4_ARRAY_TYPEDEFS
/* Macro helper types for netmod_api.h */
/* These typedefs can be removed when  */
/* we remove macro definitions of the  */
/* api */
typedef MPID_Gpid MPID_Gpid_array_t[];
typedef int intarray_t[];
typedef MPID_Request *MPID_Request_array_t[];
#endif /*MPIDI_CH4_ARRAY_TYPEDEFS */

#define USE_SHM_TYPEDEFS
#include "ch4_shm_api.h"
#undef USE_SHM_TYPEDEFS

typedef struct MPIDI_shm_funcs {
    MPIDI_shm_init_t init;
    MPIDI_shm_finalize_t finalize;
    MPIDI_shm_progress_t progress;
    MPIDI_shm_reg_hdr_handler_t reg_hdr_handler;
    MPIDI_shm_comm_connect_t comm_connect;
    MPIDI_shm_comm_disconnect_t comm_disconnect;
    MPIDI_shm_open_port_t open_port;
    MPIDI_shm_close_port_t close_port;
    MPIDI_shm_comm_accept_t comm_accept;
    MPIDI_shm_send_am_hdr_t send_am_hdr;
    MPIDI_shm_inject_am_hdr_t inject_am_hdr;
    MPIDI_shm_send_am_t send_am;
    MPIDI_shm_inject_am_t inject_am;
    MPIDI_shm_send_amv_t send_amv;
    MPIDI_shm_inject_amv_t inject_amv;
    MPIDI_shm_send_am_hdr_reply_t send_am_hdr_reply;
    MPIDI_shm_inject_am_hdr_reply_t inject_am_hdr_reply;
    MPIDI_shm_send_am_reply_t send_am_reply;
    MPIDI_shm_inject_am_reply_t inject_am_reply;
    MPIDI_shm_send_amv_reply_t send_amv_reply;
    MPIDI_shm_inject_amv_reply_t inject_amv_reply;
    MPIDI_shm_am_hdr_max_sz_t am_hdr_max_sz;
    MPIDI_shm_am_inject_max_sz_t am_inject_max_sz;
    /* Routines that handle addressing */
    MPIDI_shm_comm_get_lpid_t comm_get_lpid;
    MPIDI_shm_gpid_get_t gpid_get;
    MPIDI_shm_get_node_id_t get_node_id;
    MPIDI_shm_get_max_node_id_t get_max_node_id;
    MPIDI_shm_getallincomm_t getallincomm;
    MPIDI_shm_gpid_tolpidarray_t gpid_tolpidarray;
    MPIDI_shm_create_intercomm_from_lpids_t create_intercomm_from_lpids;
    MPIDI_shm_comm_create_t comm_create;
    MPIDI_shm_comm_destroy_t comm_destroy;
    /* Request allocation routines */
    MPIDI_shm_request_create_t request_create;
    MPIDI_shm_request_release_t request_release;
} MPIDI_shm_funcs_t;

typedef struct MPIDI_shm_native_funcs {
    MPIDI_shm_send_t send;
    MPIDI_shm_ssend_t ssend;
    MPIDI_shm_startall_t startall;
    MPIDI_shm_send_init_t send_init;
    MPIDI_shm_ssend_init_t ssend_init;
    MPIDI_shm_rsend_init_t rsend_init;
    MPIDI_shm_bsend_init_t bsend_init;
    MPIDI_shm_isend_t isend;
    MPIDI_shm_issend_t issend;
    MPIDI_shm_cancel_send_t cancel_send;
    MPIDI_shm_recv_init_t recv_init;
    MPIDI_shm_recv_t recv;
    MPIDI_shm_irecv_t irecv;
    MPIDI_shm_imrecv_t imrecv;
    MPIDI_shm_cancel_recv_t cancel_recv;
    MPIDI_shm_alloc_mem_t alloc_mem;
    MPIDI_shm_free_mem_t free_mem;
    MPIDI_shm_improbe_t improbe;
    MPIDI_shm_iprobe_t iprobe;
    MPIDI_shm_win_set_info_t win_set_info;
    MPIDI_shm_win_shared_query_t win_shared_query;
    MPIDI_shm_put_t put;
    MPIDI_shm_win_start_t win_start;
    MPIDI_shm_win_complete_t win_complete;
    MPIDI_shm_win_post_t win_post;
    MPIDI_shm_win_wait_t win_wait;
    MPIDI_shm_win_test_t win_test;
    MPIDI_shm_win_lock_t win_lock;
    MPIDI_shm_win_unlock_t win_unlock;
    MPIDI_shm_win_get_info_t win_get_info;
    MPIDI_shm_get_t get;
    MPIDI_shm_win_free_t win_free;
    MPIDI_shm_win_fence_t win_fence;
    MPIDI_shm_win_create_t win_create;
    MPIDI_shm_accumulate_t accumulate;
    MPIDI_shm_win_attach_t win_attach;
    MPIDI_shm_win_allocate_shared_t win_allocate_shared;
    MPIDI_shm_rput_t rput;
    MPIDI_shm_win_flush_local_t win_flush_local;
    MPIDI_shm_win_detach_t win_detach;
    MPIDI_shm_compare_and_swap_t compare_and_swap;
    MPIDI_shm_raccumulate_t raccumulate;
    MPIDI_shm_rget_accumulate_t rget_accumulate;
    MPIDI_shm_fetch_and_op_t fetch_and_op;
    MPIDI_shm_win_allocate_t win_allocate;
    MPIDI_shm_win_flush_t win_flush;
    MPIDI_shm_win_flush_local_all_t win_flush_local_all;
    MPIDI_shm_win_unlock_all_t win_unlock_all;
    MPIDI_shm_win_create_dynamic_t win_create_dynamic;
    MPIDI_shm_rget_t rget;
    MPIDI_shm_win_sync_t win_sync;
    MPIDI_shm_win_flush_all_t win_flush_all;
    MPIDI_shm_get_accumulate_t get_accumulate;
    MPIDI_shm_win_lock_all_t win_lock_all;
} MPIDI_shm_native_funcs_t;

extern MPIDI_shm_funcs_t *MPIDI_shm_funcs[];
extern MPIDI_shm_funcs_t *MPIDI_shm_func;
extern MPIDI_shm_native_funcs_t *MPIDI_shm_native_funcs[];
extern MPIDI_shm_native_funcs_t *MPIDI_shm_native_func;
extern int MPIDI_num_shms;
extern char MPIDI_shm_strings[][MPIDI_MAX_SHM_STRING_LEN];

#define USE_SHM_PROTOTYPES
#include "ch4_shm_api.h"
#undef USE_SHM_PROTOTYPES


#ifndef SHM_DIRECT
#ifndef SHM_DISABLE_INLINES
#define USE_SHM_INLINES
#include "ch4_shm_api.h"
#undef USE_SHM_INLINES

#endif /* SHM_DISABLE_INLINES  */

#else

#define __shm_direct_stub__     0
#define __shm_direct_simple__  1

#if SHM_DIRECT==__shm_direct_stub__
#include "../stub/ch4_shm_direct.h"
#elif SHM_DIRECT==__shm_direct_simple__
#include "../simple/ch4_shm_direct.h"
#else
#error "No direct shm included"
#endif


#endif /* SHM_DIRECT           */

#endif /* SHM_PROTOTYPES_H_INCLUDED */
