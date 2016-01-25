/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/* ch4 shm functions */
#ifndef SHM_PROTOTYPES_H_INCLUDED
#define SHM_PROTOTYPES_H_INCLUDED

#include <mpidimpl.h>

#define MPIDI_MAX_SHM_STRING_LEN 64

typedef int (*MPIDI_CH4_SHM_am_completion_handler_fn) (MPID_Request * req);
typedef int (*MPIDI_CH4_SHM_am_origin_handler_fn) (MPID_Request * req);

/* Callback function setup by handler register function */
/* for short cases, output arguments are NULL */
typedef int (*MPIDI_CH4_SHM_am_target_handler_fn)
 (void *am_hdr, size_t am_hdr_sz, uint64_t reply_token,    /* contains information about reply operation */
  void **data,                  /* CH4 manages this buffer - shm only fills with data */
  MPI_Datatype * datatype, MPI_Count * count, int *noncontig,   /* if TRUE: data/data_sz are actually iovec/count */
  MPIDI_CH4_SHM_am_completion_handler_fn * cmpl_handler_fn, /* completion handler */
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

typedef struct MPIDI_CH4_SHM_funcs {
    MPIDI_CH4_SHM_init_t init;
    MPIDI_CH4_SHM_finalize_t finalize;
    MPIDI_CH4_SHM_progress_t progress;
    MPIDI_CH4_SHM_reg_hdr_handler_t reg_hdr_handler;
    MPIDI_CH4_SHM_comm_connect_t comm_connect;
    MPIDI_CH4_SHM_comm_disconnect_t comm_disconnect;
    MPIDI_CH4_SHM_open_port_t open_port;
    MPIDI_CH4_SHM_close_port_t close_port;
    MPIDI_CH4_SHM_comm_accept_t comm_accept;
    MPIDI_CH4_SHM_send_am_hdr_t send_am_hdr;
    MPIDI_CH4_SHM_inject_am_hdr_t inject_am_hdr;
    MPIDI_CH4_SHM_send_am_t send_am;
    MPIDI_CH4_SHM_inject_am_t inject_am;
    MPIDI_CH4_SHM_send_amv_t send_amv;
    MPIDI_CH4_SHM_inject_amv_t inject_amv;
    MPIDI_CH4_SHM_send_am_hdr_reply_t send_am_hdr_reply;
    MPIDI_CH4_SHM_inject_am_hdr_reply_t inject_am_hdr_reply;
    MPIDI_CH4_SHM_send_am_reply_t send_am_reply;
    MPIDI_CH4_SHM_inject_am_reply_t inject_am_reply;
    MPIDI_CH4_SHM_send_amv_reply_t send_amv_reply;
    MPIDI_CH4_SHM_inject_amv_reply_t inject_amv_reply;
    MPIDI_CH4_SHM_am_hdr_max_sz_t am_hdr_max_sz;
    MPIDI_CH4_SHM_am_inject_max_sz_t am_inject_max_sz;
    /* Routines that handle addressing */
    MPIDI_CH4_SHM_comm_get_lpid_t comm_get_lpid;
    MPIDI_CH4_SHM_gpid_get_t gpid_get;
    MPIDI_CH4_SHM_get_node_id_t get_node_id;
    MPIDI_CH4_SHM_get_max_node_id_t get_max_node_id;
    MPIDI_CH4_SHM_getallincomm_t getallincomm;
    MPIDI_CH4_SHM_gpid_tolpidarray_t gpid_tolpidarray;
    MPIDI_CH4_SHM_create_intercomm_from_lpids_t create_intercomm_from_lpids;
    MPIDI_CH4_SHM_comm_create_t comm_create;
    MPIDI_CH4_SHM_comm_destroy_t comm_destroy;
    /* Request allocation routines */
    MPIDI_CH4_SHM_am_request_init_t am_request_init;
    MPIDI_CH4_SHM_am_request_finalize_t am_request_finalize;
} MPIDI_CH4_SHM_funcs_t;

typedef struct MPIDI_CH4_SHM_native_funcs {
    MPIDI_CH4_SHM_send_t send;
    MPIDI_CH4_SHM_ssend_t ssend;
    MPIDI_CH4_SHM_startall_t startall;
    MPIDI_CH4_SHM_send_init_t send_init;
    MPIDI_CH4_SHM_ssend_init_t ssend_init;
    MPIDI_CH4_SHM_rsend_init_t rsend_init;
    MPIDI_CH4_SHM_bsend_init_t bsend_init;
    MPIDI_CH4_SHM_isend_t isend;
    MPIDI_CH4_SHM_issend_t issend;
    MPIDI_CH4_SHM_cancel_send_t cancel_send;
    MPIDI_CH4_SHM_recv_init_t recv_init;
    MPIDI_CH4_SHM_recv_t recv;
    MPIDI_CH4_SHM_irecv_t irecv;
    MPIDI_CH4_SHM_imrecv_t imrecv;
    MPIDI_CH4_SHM_cancel_recv_t cancel_recv;
    MPIDI_CH4_SHM_alloc_mem_t alloc_mem;
    MPIDI_CH4_SHM_free_mem_t free_mem;
    MPIDI_CH4_SHM_improbe_t improbe;
    MPIDI_CH4_SHM_iprobe_t iprobe;
    MPIDI_CH4_SHM_win_set_info_t win_set_info;
    MPIDI_CH4_SHM_win_shared_query_t win_shared_query;
    MPIDI_CH4_SHM_put_t put;
    MPIDI_CH4_SHM_win_start_t win_start;
    MPIDI_CH4_SHM_win_complete_t win_complete;
    MPIDI_CH4_SHM_win_post_t win_post;
    MPIDI_CH4_SHM_win_wait_t win_wait;
    MPIDI_CH4_SHM_win_test_t win_test;
    MPIDI_CH4_SHM_win_lock_t win_lock;
    MPIDI_CH4_SHM_win_unlock_t win_unlock;
    MPIDI_CH4_SHM_win_get_info_t win_get_info;
    MPIDI_CH4_SHM_get_t get;
    MPIDI_CH4_SHM_win_free_t win_free;
    MPIDI_CH4_SHM_win_fence_t win_fence;
    MPIDI_CH4_SHM_win_create_t win_create;
    MPIDI_CH4_SHM_accumulate_t accumulate;
    MPIDI_CH4_SHM_win_attach_t win_attach;
    MPIDI_CH4_SHM_win_allocate_shared_t win_allocate_shared;
    MPIDI_CH4_SHM_rput_t rput;
    MPIDI_CH4_SHM_win_flush_local_t win_flush_local;
    MPIDI_CH4_SHM_win_detach_t win_detach;
    MPIDI_CH4_SHM_compare_and_swap_t compare_and_swap;
    MPIDI_CH4_SHM_raccumulate_t raccumulate;
    MPIDI_CH4_SHM_rget_accumulate_t rget_accumulate;
    MPIDI_CH4_SHM_fetch_and_op_t fetch_and_op;
    MPIDI_CH4_SHM_win_allocate_t win_allocate;
    MPIDI_CH4_SHM_win_flush_t win_flush;
    MPIDI_CH4_SHM_win_flush_local_all_t win_flush_local_all;
    MPIDI_CH4_SHM_win_unlock_all_t win_unlock_all;
    MPIDI_CH4_SHM_win_create_dynamic_t win_create_dynamic;
    MPIDI_CH4_SHM_rget_t rget;
    MPIDI_CH4_SHM_win_sync_t win_sync;
    MPIDI_CH4_SHM_win_flush_all_t win_flush_all;
    MPIDI_CH4_SHM_get_accumulate_t get_accumulate;
    MPIDI_CH4_SHM_win_lock_all_t win_lock_all;
} MPIDI_CH4_SHM_native_funcs_t;

extern MPIDI_CH4_SHM_funcs_t *MPIDI_CH4_SHM_funcs[];
extern MPIDI_CH4_SHM_funcs_t *MPIDI_CH4_SHM_func;
extern MPIDI_CH4_SHM_native_funcs_t *MPIDI_CH4_SHM_native_funcs[];
extern MPIDI_CH4_SHM_native_funcs_t *MPIDI_CH4_SHM_native_func;
extern int MPIDI_num_shms;
extern char MPIDI_CH4_SHM_strings[][MPIDI_MAX_SHM_STRING_LEN];

#define USE_SHM_PROTOTYPES
#include "ch4_shm_api.h"
#undef USE_SHM_PROTOTYPES

#endif /* SHM_PROTOTYPES_H_INCLUDED */
