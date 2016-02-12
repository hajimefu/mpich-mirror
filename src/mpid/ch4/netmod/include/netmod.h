/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/* ch4 netmod functions */
#ifndef NETMOD_PROTOTYPES_H_INCLUDED
#define NETMOD_PROTOTYPES_H_INCLUDED

#include <mpidimpl.h>

#define MPIDI_MAX_NETMOD_STRING_LEN 64

typedef int (*MPIDI_CH4_NM_am_completion_handler_fn) (MPID_Request * req);
typedef int (*MPIDI_CH4_NM_am_origin_handler_fn) (MPID_Request * req);

/* Callback function setup by handler register function */
/* for short cases, output arguments are NULL */
typedef int (*MPIDI_CH4_NM_am_target_handler_fn)
 (void                                   *am_hdr,
  size_t                                  am_hdr_sz,
  uint64_t                                reply_token,    /* contains information about reply operation */
  void                                  **data,           /* data should be set as iovs if *is_contig is true */
  size_t                                 *data_sz,
  int                                    *is_contig,
  MPIDI_CH4_NM_am_completion_handler_fn  *cmpl_handler_fn,/* completion handler */
  MPID_Request                          **req);           /* if allocated, need pointer to completion function */

#ifndef MPIDI_CH4_ARRAY_TYPEDEFS
#define MPIDI_CH4_ARRAY_TYPEDEFS
/* Macro helper types for netmod_api.h */
/* These typedefs can be removed when  */
/* we remove macro definitions of the  */
/* api */
typedef MPID_Gpid MPID_Gpid_array_t[];
typedef int intarray_t[];
typedef MPID_Request *MPID_Request_array_t[];
typedef MPI_Datatype MPI_Datatype_array_t[];
#endif /*MPIDI_CH4_ARRAY_TYPEDEFS */

#define USE_NETMOD_TYPEDEFS
#include "netmod_api.h"
#undef USE_NETMOD_TYPEDEFS

typedef struct MPIDI_CH4_NM_funcs {
    MPIDI_CH4_NM_init_t init;
    MPIDI_CH4_NM_finalize_t finalize;
    MPIDI_CH4_NM_progress_t progress;
    MPIDI_CH4_NM_comm_connect_t comm_connect;
    MPIDI_CH4_NM_comm_disconnect_t comm_disconnect;
    MPIDI_CH4_NM_open_port_t open_port;
    MPIDI_CH4_NM_close_port_t close_port;
    MPIDI_CH4_NM_comm_accept_t comm_accept;
    /* Routines that handle addressing */
    MPIDI_CH4_NM_comm_get_lpid_t comm_get_lpid;
    MPIDI_CH4_NM_gpid_get_t gpid_get;
    MPIDI_CH4_NM_get_node_id_t get_node_id;
    MPIDI_CH4_NM_get_max_node_id_t get_max_node_id;
    MPIDI_CH4_NM_getallincomm_t getallincomm;
    MPIDI_CH4_NM_gpid_tolpidarray_t gpid_tolpidarray;
    MPIDI_CH4_NM_create_intercomm_from_lpids_t create_intercomm_from_lpids;
    MPIDI_CH4_NM_comm_create_t comm_create;
    MPIDI_CH4_NM_comm_destroy_t comm_destroy;
    /* Request allocation routines */
    MPIDI_CH4_NM_am_request_init_t am_request_init;
    MPIDI_CH4_NM_am_request_finalize_t am_request_finalize;
    /* Active Message Routines */
    MPIDI_CH4_NM_reg_hdr_handler_t reg_hdr_handler;
    MPIDI_CH4_NM_send_am_hdr_t send_am_hdr;
    MPIDI_CH4_NM_inject_am_hdr_t inject_am_hdr;
    MPIDI_CH4_NM_send_am_t send_am;
    MPIDI_CH4_NM_send_amv_t send_amv;
    MPIDI_CH4_NM_send_amv_hdr_t send_amv_hdr;
    MPIDI_CH4_NM_send_am_hdr_reply_t send_am_hdr_reply;
    MPIDI_CH4_NM_inject_am_hdr_reply_t inject_am_hdr_reply;
    MPIDI_CH4_NM_send_am_reply_t send_am_reply;
    MPIDI_CH4_NM_send_amv_reply_t send_amv_reply;
    MPIDI_CH4_NM_am_hdr_max_sz_t am_hdr_max_sz;
    MPIDI_CH4_NM_am_inject_max_sz_t am_inject_max_sz;

} MPIDI_CH4_NM_funcs_t;

typedef struct MPIDI_CH4_NM_native_funcs {
    MPIDI_CH4_NM_send_t send;
    MPIDI_CH4_NM_ssend_t ssend;
    MPIDI_CH4_NM_startall_t startall;
    MPIDI_CH4_NM_send_init_t send_init;
    MPIDI_CH4_NM_ssend_init_t ssend_init;
    MPIDI_CH4_NM_rsend_init_t rsend_init;
    MPIDI_CH4_NM_bsend_init_t bsend_init;
    MPIDI_CH4_NM_isend_t isend;
    MPIDI_CH4_NM_issend_t issend;
    MPIDI_CH4_NM_cancel_send_t cancel_send;
    MPIDI_CH4_NM_recv_init_t recv_init;
    MPIDI_CH4_NM_recv_t recv;
    MPIDI_CH4_NM_irecv_t irecv;
    MPIDI_CH4_NM_imrecv_t imrecv;
    MPIDI_CH4_NM_cancel_recv_t cancel_recv;
    MPIDI_CH4_NM_alloc_mem_t alloc_mem;
    MPIDI_CH4_NM_free_mem_t free_mem;
    MPIDI_CH4_NM_improbe_t improbe;
    MPIDI_CH4_NM_iprobe_t iprobe;
    MPIDI_CH4_NM_win_set_info_t win_set_info;
    MPIDI_CH4_NM_win_shared_query_t win_shared_query;
    MPIDI_CH4_NM_put_t put;
    MPIDI_CH4_NM_win_start_t win_start;
    MPIDI_CH4_NM_win_complete_t win_complete;
    MPIDI_CH4_NM_win_post_t win_post;
    MPIDI_CH4_NM_win_wait_t win_wait;
    MPIDI_CH4_NM_win_test_t win_test;
    MPIDI_CH4_NM_win_lock_t win_lock;
    MPIDI_CH4_NM_win_unlock_t win_unlock;
    MPIDI_CH4_NM_win_get_info_t win_get_info;
    MPIDI_CH4_NM_get_t get;
    MPIDI_CH4_NM_win_free_t win_free;
    MPIDI_CH4_NM_win_fence_t win_fence;
    MPIDI_CH4_NM_win_create_t win_create;
    MPIDI_CH4_NM_accumulate_t accumulate;
    MPIDI_CH4_NM_win_attach_t win_attach;
    MPIDI_CH4_NM_win_allocate_shared_t win_allocate_shared;
    MPIDI_CH4_NM_rput_t rput;
    MPIDI_CH4_NM_win_flush_local_t win_flush_local;
    MPIDI_CH4_NM_win_detach_t win_detach;
    MPIDI_CH4_NM_compare_and_swap_t compare_and_swap;
    MPIDI_CH4_NM_raccumulate_t raccumulate;
    MPIDI_CH4_NM_rget_accumulate_t rget_accumulate;
    MPIDI_CH4_NM_fetch_and_op_t fetch_and_op;
    MPIDI_CH4_NM_win_allocate_t win_allocate;
    MPIDI_CH4_NM_win_flush_t win_flush;
    MPIDI_CH4_NM_win_flush_local_all_t win_flush_local_all;
    MPIDI_CH4_NM_win_unlock_all_t win_unlock_all;
    MPIDI_CH4_NM_win_create_dynamic_t win_create_dynamic;
    MPIDI_CH4_NM_rget_t rget;
    MPIDI_CH4_NM_win_sync_t win_sync;
    MPIDI_CH4_NM_win_flush_all_t win_flush_all;
    MPIDI_CH4_NM_get_accumulate_t get_accumulate;
    MPIDI_CH4_NM_win_lock_all_t win_lock_all;
    MPIDI_CH4_NM_rank_is_local_t rank_is_local;
    /* Collectives */
    MPIDI_CH4_NM_barrier_t barrier;
    MPIDI_CH4_NM_bcast_t bcast;
    MPIDI_CH4_NM_allreduce_t allreduce;
    MPIDI_CH4_NM_allgather_t allgather;
    MPIDI_CH4_NM_allgatherv_t allgatherv;
    MPIDI_CH4_NM_scatter_t scatter;
    MPIDI_CH4_NM_scatterv_t scatterv;
    MPIDI_CH4_NM_gather_t gather;
    MPIDI_CH4_NM_gatherv_t gatherv;
    MPIDI_CH4_NM_alltoall_t alltoall;
    MPIDI_CH4_NM_alltoallv_t alltoallv;
    MPIDI_CH4_NM_alltoallw_t alltoallw;
    MPIDI_CH4_NM_reduce_t reduce;
    MPIDI_CH4_NM_reduce_scatter_t reduce_scatter;
    MPIDI_CH4_NM_reduce_scatter_block_t reduce_scatter_block;
    MPIDI_CH4_NM_scan_t scan;
    MPIDI_CH4_NM_exscan_t exscan;
    MPIDI_CH4_NM_neighbor_allgather_t neighbor_allgather;
    MPIDI_CH4_NM_neighbor_allgatherv_t neighbor_allgatherv;
    MPIDI_CH4_NM_neighbor_alltoall_t neighbor_alltoall;
    MPIDI_CH4_NM_neighbor_alltoallv_t neighbor_alltoallv;
    MPIDI_CH4_NM_neighbor_alltoallw_t neighbor_alltoallw;
    MPIDI_CH4_NM_ineighbor_allgather_t ineighbor_allgather;
    MPIDI_CH4_NM_ineighbor_allgatherv_t ineighbor_allgatherv;
    MPIDI_CH4_NM_ineighbor_alltoall_t ineighbor_alltoall;
    MPIDI_CH4_NM_ineighbor_alltoallv_t ineighbor_alltoallv;
    MPIDI_CH4_NM_ineighbor_alltoallw_t ineighbor_alltoallw;
    MPIDI_CH4_NM_ibarrier_t ibarrier;
    MPIDI_CH4_NM_ibcast_t ibcast;
    MPIDI_CH4_NM_iallgather_t iallgather;
    MPIDI_CH4_NM_iallgatherv_t iallgatherv;
    MPIDI_CH4_NM_iallreduce_t iallreduce;
    MPIDI_CH4_NM_ialltoall_t ialltoall;
    MPIDI_CH4_NM_ialltoallv_t ialltoallv;
    MPIDI_CH4_NM_ialltoallw_t ialltoallw;
    MPIDI_CH4_NM_iexscan_t iexscan;
    MPIDI_CH4_NM_igather_t igather;
    MPIDI_CH4_NM_igatherv_t igatherv;
    MPIDI_CH4_NM_ireduce_scatter_block_t ireduce_scatter_block;
    MPIDI_CH4_NM_ireduce_scatter_t ireduce_scatter;
    MPIDI_CH4_NM_ireduce_t ireduce;
    MPIDI_CH4_NM_iscan_t iscan;
    MPIDI_CH4_NM_iscatter_t iscatter;
    MPIDI_CH4_NM_iscatterv_t iscatterv;
} MPIDI_CH4_NM_native_funcs_t;

extern MPIDI_CH4_NM_funcs_t *MPIDI_CH4_NM_funcs[];
extern MPIDI_CH4_NM_funcs_t *MPIDI_CH4_NM_func;
extern MPIDI_CH4_NM_native_funcs_t *MPIDI_CH4_NM_native_funcs[];
extern MPIDI_CH4_NM_native_funcs_t *MPIDI_CH4_NM_native_func;
extern int MPIDI_num_netmods;
extern char MPIDI_CH4_NM_strings[][MPIDI_MAX_NETMOD_STRING_LEN];


#define USE_NETMOD_PROTOTYPES
#include "netmod_api.h"
#undef USE_NETMOD_PROTOTYPES


#endif
