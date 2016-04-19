/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
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
 (void *am_hdr, size_t am_hdr_sz,
  void **data,                  /* CH4 manages this buffer - shm only fills with data */
  MPI_Datatype * datatype, MPI_Count * count, int *noncontig,   /* if TRUE: data/data_sz are actually iovec/count */
  MPIDI_CH4_SHM_am_completion_handler_fn * cmpl_handler_fn, /* completion handler */
  MPID_Request ** req);         /* if allocated, need pointer to completion function */

typedef int (*MPIDI_CH4_SHM_init_t)(int rank, int size);
typedef int (*MPIDI_CH4_SHM_finalize_t)(void);
typedef int (*MPIDI_CH4_SHM_progress_t)(int blocking);
typedef int (*MPIDI_CH4_SHM_reg_hdr_handler_t)(int handler_id, MPIDI_CH4_SHM_am_origin_handler_fn origin_handler_fn, MPIDI_CH4_SHM_am_target_handler_fn target_handler_fn);
typedef int (*MPIDI_CH4_SHM_comm_connect_t)(const char * port_name, MPID_Info * info, int root, MPID_Comm * comm, MPID_Comm ** newcomm_ptr);
typedef int (*MPIDI_CH4_SHM_comm_disconnect_t)(MPID_Comm * comm_ptr);
typedef int (*MPIDI_CH4_SHM_open_port_t)(MPID_Info * info_ptr, char * port_name);
typedef int (*MPIDI_CH4_SHM_close_port_t)(const char * port_name);
typedef int (*MPIDI_CH4_SHM_comm_accept_t)(const char * port_name, MPID_Info * info, int root, MPID_Comm * comm, MPID_Comm ** newcomm_ptr);
typedef int (*MPIDI_CH4_SHM_send_am_hdr_t)(int rank, MPID_Comm * comm, int handler_id, const void* am_hdr, size_t am_hdr_sz, MPID_Request * sreq, void * shm_context);
typedef int (*MPIDI_CH4_SHM_inject_am_hdr_t)(int rank, MPID_Comm * comm, int handler_id, const void* am_hdr, size_t am_hdr_sz, void * shm_context);
typedef int (*MPIDI_CH4_SHM_send_am_t)(int rank, MPID_Comm * comm, int handler_id, const void * am_hdr, size_t am_hdr_sz, const void * data, MPI_Count count, MPI_Datatype datatype, MPID_Request * sreq, void * shm_context);
typedef int (*MPIDI_CH4_SHM_inject_am_t)(int rank, MPID_Comm * comm, int handler_id, const void * am_hdr, size_t am_hdr_sz, const void * data, MPI_Count count, MPI_Datatype datatype, void * shm_context);
typedef int (*MPIDI_CH4_SHM_send_amv_t)(int rank, MPID_Comm * comm, int handler_id, struct iovec * am_hdrs, size_t iov_len, const void * data, MPI_Count count, MPI_Datatype datatype, MPID_Request * sreq, void * shm_context);
typedef int (*MPIDI_CH4_SHM_inject_amv_t)(int rank, MPID_Comm * comm, int handler_id, struct iovec * am_hdrs, size_t iov_len, const void * data, MPI_Count count, MPI_Datatype datatype, void * shm_context);
typedef int (*MPIDI_CH4_SHM_send_am_hdr_reply_t)(MPIU_Context_id_t context_id, int src_rank, int handler_id, const void * am_hdr, size_t am_hdr_sz, MPID_Request * sreq);
typedef int (*MPIDI_CH4_SHM_inject_am_hdr_reply_t)(MPIU_Context_id_t context_id, int src_rank, int handler_id, const void * am_hdr, size_t am_hdr_sz);
typedef int (*MPIDI_CH4_SHM_send_am_reply_t)(MPIU_Context_id_t context_id, int src_rank, int handler_id, const void * am_hdr, size_t am_hdr_sz, const void * data, MPI_Count count, MPI_Datatype datatype, MPID_Request * sreq);
typedef int (*MPIDI_CH4_SHM_inject_am_reply_t)(MPIU_Context_id_t context_id, int src_rank, int handler_id, const void * am_hdr, size_t am_hdr_sz, const void * data, MPI_Count count, MPI_Datatype datatype);
typedef int (*MPIDI_CH4_SHM_send_amv_reply_t)(MPIU_Context_id_t context_id, int src_rank, int handler_id, struct iovec * am_hdr, size_t iov_len, const void * data, MPI_Count count, MPI_Datatype datatype, MPID_Request * sreq);
typedef int (*MPIDI_CH4_SHM_inject_amv_reply_t)(MPIU_Context_id_t context_id, int src_rank, int handler_id, struct iovec * am_hdrs, size_t iov_len, const void * data, MPI_Count count, MPI_Datatype datatype);
typedef size_t (*MPIDI_CH4_SHM_am_hdr_max_sz_t)(void);
typedef size_t (*MPIDI_CH4_SHM_am_inject_max_sz_t)(void);
typedef int (*MPIDI_CH4_SHM_comm_get_lpid_t)(MPID_Comm * comm_ptr, int idx, int * lpid_ptr, MPIU_BOOL is_remote);
typedef int (*MPIDI_CH4_SHM_gpid_get_t)(MPID_Comm * comm_ptr, int rank, MPIR_Gpid * gpid);
typedef int (*MPIDI_CH4_SHM_get_node_id_t)(MPID_Comm * comm, int rank, MPID_Node_id_t * id_p);
typedef int (*MPIDI_CH4_SHM_get_max_node_id_t)(MPID_Comm * comm, MPID_Node_id_t * max_id_p);
typedef int (*MPIDI_CH4_SHM_getallincomm_t)(MPID_Comm * comm_ptr, int local_size, MPIR_Gpid local_gpid[], int * singlePG);
typedef int (*MPIDI_CH4_SHM_gpid_tolpidarray_t)(int size, MPIR_Gpid gpid[], int lpid[]);
typedef int (*MPIDI_CH4_SHM_create_intercomm_from_lpids_t)(MPID_Comm * newcomm_ptr, int size, const int lpids[]);
typedef int (*MPIDI_CH4_SHM_comm_create_t)(MPID_Comm * comm);
typedef int (*MPIDI_CH4_SHM_comm_destroy_t)(MPID_Comm * comm);
typedef void (*MPIDI_CH4_SHM_am_request_init_t)(MPID_Request * req);
typedef void (*MPIDI_CH4_SHM_am_request_finalize_t)(MPID_Request * req);
typedef int (*MPIDI_CH4_SHM_send_t)(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_ssend_t)(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_startall_t)(int count, MPID_Request *requests[]);
typedef int (*MPIDI_CH4_SHM_send_init_t)(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_ssend_init_t)(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_rsend_init_t)(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_bsend_init_t)(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_isend_t)(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_issend_t)(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_cancel_send_t)(MPID_Request * sreq);
typedef int (*MPIDI_CH4_SHM_recv_init_t)(void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_recv_t)(void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPI_Status * status, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_irecv_t)(void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_imrecv_t)(void * buf, int count, MPI_Datatype datatype, MPID_Request * message, MPID_Request ** rreqp);
typedef int (*MPIDI_CH4_SHM_cancel_recv_t)(MPID_Request * rreq);
typedef void * (*MPIDI_CH4_SHM_alloc_mem_t)(size_t size, MPID_Info * info_ptr);
typedef int (*MPIDI_CH4_SHM_free_mem_t)(void *ptr);
typedef int (*MPIDI_CH4_SHM_improbe_t)(int source, int tag, MPID_Comm * comm, int context_offset, int *flag, MPID_Request ** message, MPI_Status * status);
typedef int (*MPIDI_CH4_SHM_iprobe_t)(int source, int tag, MPID_Comm * comm, int context_offset, int * flag, MPI_Status * status);
typedef int (*MPIDI_CH4_SHM_win_set_info_t)(MPID_Win * win, MPID_Info * info);
typedef int (*MPIDI_CH4_SHM_win_shared_query_t)(MPID_Win * win, int rank, MPI_Aint * size, int * disp_unit, void * baseptr);
typedef int (*MPIDI_CH4_SHM_put_t)(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_start_t)(MPID_Group * group, int assert, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_complete_t)(MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_post_t)(MPID_Group * group, int assert, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_wait_t)(MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_test_t)(MPID_Win * win, int * flag);
typedef int (*MPIDI_CH4_SHM_win_lock_t)(int lock_type, int rank, int assert, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_unlock_t)(int rank, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_get_info_t)(MPID_Win * win, MPID_Info ** info_p_p);
typedef int (*MPIDI_CH4_SHM_get_t)(void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_free_t)(MPID_Win ** win_ptr);
typedef int (*MPIDI_CH4_SHM_win_fence_t)(int assert, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_create_t)(void * base, MPI_Aint length, int disp_unit, MPID_Info * info, MPID_Comm * comm_ptr, MPID_Win ** win_ptr);
typedef int (*MPIDI_CH4_SHM_accumulate_t)(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_attach_t)(MPID_Win * win, void * base, MPI_Aint size);
typedef int (*MPIDI_CH4_SHM_win_allocate_shared_t)(MPI_Aint size, int disp_unit, MPID_Info * info_ptr, MPID_Comm * comm_ptr, void ** base_ptr, MPID_Win ** win_ptr);
typedef int (*MPIDI_CH4_SHM_rput_t)(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPID_Win * win, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_win_flush_local_t)(int rank, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_detach_t)(MPID_Win * win, const void * base);
typedef int (*MPIDI_CH4_SHM_compare_and_swap_t)(const void * origin_addr, const void * compare_addr, void * result_addr, MPI_Datatype datatype, int target_rank, MPI_Aint target_disp, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_raccumulate_t)(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_rget_accumulate_t)(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, void * result_addr, int result_count, MPI_Datatype result_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_fetch_and_op_t)(const void * origin_addr, void * result_addr, MPI_Datatype datatype, int target_rank, MPI_Aint target_disp, MPI_Op op, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_allocate_t)(MPI_Aint size, int disp_unit, MPID_Info * info, MPID_Comm * comm, void * baseptr, MPID_Win ** win);
typedef int (*MPIDI_CH4_SHM_win_flush_t)(int rank, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_flush_local_all_t)(MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_unlock_all_t)(MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_create_dynamic_t)(MPID_Info * info, MPID_Comm * comm, MPID_Win ** win);
typedef int (*MPIDI_CH4_SHM_rget_t)(void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPID_Win * win, MPID_Request ** request);
typedef int (*MPIDI_CH4_SHM_win_sync_t)(MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_flush_all_t)(MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_get_accumulate_t)(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, void * result_addr, int result_count, MPI_Datatype result_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_win_lock_all_t)(int assert, MPID_Win * win);
typedef int (*MPIDI_CH4_SHM_barrier_t)(MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_bcast_t)(void* buffer, int count, MPI_Datatype datatype, int root, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_allreduce_t)(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_allgather_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_allgatherv_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_scatter_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_scatterv_t)(const void * sendbuf, const int * sendcounts, const int * displs, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_gather_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_gatherv_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_alltoall_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_alltoallv_t)(const void * sendbuf, const int * sendcounts, const int * sdispls, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * rdispls, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_alltoallw_t)(const void * sendbuf, const int * sendcounts, const int * sdispls, const MPI_Datatype sendtypes[], void * recvbuf, const int * recvcounts, const int * rdispls, const MPI_Datatype recvtypes[], MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_reduce_t)(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_reduce_scatter_t)(const void * sendbuf, void * recvbuf, const int * recvcounts, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_reduce_scatter_block_t)(const void * sendbuf, void * recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_scan_t)(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_exscan_t)(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_neighbor_allgather_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_neighbor_allgatherv_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_neighbor_alltoallv_t)(const void * sendbuf, const int * sendcounts, const int * sdispls, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * rdispls, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_neighbor_alltoallw_t)(const void * sendbuf, const int * sendcounts, const MPI_Aint * sdispls, const MPI_Datatype * sendtypes, void * recvbuf, const int * recvcounts, const MPI_Aint * rdispls, const MPI_Datatype * recvtypes, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_neighbor_alltoall_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag);
typedef int (*MPIDI_CH4_SHM_ineighbor_allgather_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_ineighbor_allgatherv_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_ineighbor_alltoall_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_ineighbor_alltoallv_t)(const void * sendbuf, const int * sendcounts, const int * sdispls, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * rdispls, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_ineighbor_alltoallw_t)(const void * sendbuf, const int * sendcounts, const MPI_Aint * sdispls, const MPI_Datatype * sendtypes, void * recvbuf, const int * recvcounts, const MPI_Aint * rdispls, const MPI_Datatype * recvtypes, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_ibarrier_t)(MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_ibcast_t)(void* buffer, int count, MPI_Datatype datatype, int root, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_iallgather_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_iallgatherv_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_iallreduce_t)(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_ialltoall_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_ialltoallv_t)(const void * sendbuf, const int * sendcounts, const int * sdispls, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * rdispls, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_ialltoallw_t)(const void * sendbuf, const int * sendcounts, const int * sdispls, const MPI_Datatype sendtypes[], void * recvbuf, const int * recvcounts, const int * rdispls, const MPI_Datatype recvtypes[], MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_iexscan_t)(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_igather_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_igatherv_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_ireduce_scatter_block_t)(const void * sendbuf, void * recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_ireduce_scatter_t)(const void * sendbuf, void * recvbuf, const int * recvcounts, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_ireduce_t)(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPID_Comm * comm_ptr, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_iscan_t)(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_iscatter_t)(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPI_Request * req);
typedef int (*MPIDI_CH4_SHM_iscatterv_t)(const void * sendbuf, const int * sendcounts, const int * displs, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm_ptr, MPI_Request * req);

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
    /* Collectives */
    MPIDI_CH4_SHM_barrier_t barrier;
    MPIDI_CH4_SHM_bcast_t bcast;
    MPIDI_CH4_SHM_allreduce_t allreduce;
    MPIDI_CH4_SHM_allgather_t allgather;
    MPIDI_CH4_SHM_allgatherv_t allgatherv;
    MPIDI_CH4_SHM_scatter_t scatter;
    MPIDI_CH4_SHM_scatterv_t scatterv;
    MPIDI_CH4_SHM_gather_t gather;
    MPIDI_CH4_SHM_gatherv_t gatherv;
    MPIDI_CH4_SHM_alltoall_t alltoall;
    MPIDI_CH4_SHM_alltoallv_t alltoallv;
    MPIDI_CH4_SHM_alltoallw_t alltoallw;
    MPIDI_CH4_SHM_reduce_t reduce;
    MPIDI_CH4_SHM_reduce_scatter_t reduce_scatter;
    MPIDI_CH4_SHM_reduce_scatter_block_t reduce_scatter_block;
    MPIDI_CH4_SHM_scan_t scan;
    MPIDI_CH4_SHM_exscan_t exscan;
    MPIDI_CH4_SHM_neighbor_allgather_t neighbor_allgather;
    MPIDI_CH4_SHM_neighbor_allgatherv_t neighbor_allgatherv;
    MPIDI_CH4_SHM_neighbor_alltoall_t neighbor_alltoall;
    MPIDI_CH4_SHM_neighbor_alltoallv_t neighbor_alltoallv;
    MPIDI_CH4_SHM_neighbor_alltoallw_t neighbor_alltoallw;
    MPIDI_CH4_SHM_ineighbor_allgather_t ineighbor_allgather;
    MPIDI_CH4_SHM_ineighbor_allgatherv_t ineighbor_allgatherv;
    MPIDI_CH4_SHM_ineighbor_alltoall_t ineighbor_alltoall;
    MPIDI_CH4_SHM_ineighbor_alltoallv_t ineighbor_alltoallv;
    MPIDI_CH4_SHM_ineighbor_alltoallw_t ineighbor_alltoallw;
    MPIDI_CH4_SHM_ibarrier_t ibarrier;
    MPIDI_CH4_SHM_ibcast_t ibcast;
    MPIDI_CH4_SHM_iallgather_t iallgather;
    MPIDI_CH4_SHM_iallgatherv_t iallgatherv;
    MPIDI_CH4_SHM_iallreduce_t iallreduce;
    MPIDI_CH4_SHM_ialltoall_t ialltoall;
    MPIDI_CH4_SHM_ialltoallv_t ialltoallv;
    MPIDI_CH4_SHM_ialltoallw_t ialltoallw;
    MPIDI_CH4_SHM_iexscan_t iexscan;
    MPIDI_CH4_SHM_igather_t igather;
    MPIDI_CH4_SHM_igatherv_t igatherv;
    MPIDI_CH4_SHM_ireduce_scatter_block_t ireduce_scatter_block;
    MPIDI_CH4_SHM_ireduce_scatter_t ireduce_scatter;
    MPIDI_CH4_SHM_ireduce_t ireduce;
    MPIDI_CH4_SHM_iscan_t iscan;
    MPIDI_CH4_SHM_iscatter_t iscatter;
    MPIDI_CH4_SHM_iscatterv_t iscatterv;
} MPIDI_CH4_SHM_native_funcs_t;

extern MPIDI_CH4_SHM_funcs_t *MPIDI_CH4_SHM_funcs[];
extern MPIDI_CH4_SHM_funcs_t *MPIDI_CH4_SHM_func;
extern MPIDI_CH4_SHM_native_funcs_t *MPIDI_CH4_SHM_native_funcs[];
extern MPIDI_CH4_SHM_native_funcs_t *MPIDI_CH4_SHM_native_func;
extern int MPIDI_num_shms;
extern char MPIDI_CH4_SHM_strings[][MPIDI_MAX_SHM_STRING_LEN];

#ifndef MPIDI_CH4_SHM_STATIC_INLINE_PREFIX
#define MPIDI_CH4_SHM_STATIC_INLINE_PREFIX __attribute__((always_inline)) static inline
#endif

#ifndef MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX
#define MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX __attribute__((always_inline))
#endif

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_init(int rank, int size) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_finalize(void) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_progress(int blocking) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_reg_hdr_handler(int handler_id, MPIDI_CH4_SHM_am_origin_handler_fn origin_handler_fn, MPIDI_CH4_SHM_am_target_handler_fn target_handler_fn) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_comm_connect(const char * port_name, MPID_Info * info, int root, MPID_Comm * comm, MPID_Comm ** newcomm_ptr) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_comm_disconnect(MPID_Comm * comm_ptr) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_open_port(MPID_Info * info_ptr, char * port_name) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_close_port(const char * port_name) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_comm_accept(const char * port_name, MPID_Info * info, int root, MPID_Comm * comm, MPID_Comm ** newcomm_ptr) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_am_hdr(int rank, MPID_Comm * comm, int handler_id, const void* am_hdr, size_t am_hdr_sz, MPID_Request * sreq, void * shm_context) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_inject_am_hdr(int rank, MPID_Comm * comm, int handler_id, const void* am_hdr, size_t am_hdr_sz, void * shm_context) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_am(int rank, MPID_Comm * comm, int handler_id, const void * am_hdr, size_t am_hdr_sz, const void * data, MPI_Count count, MPI_Datatype datatype, MPID_Request * sreq, void * shm_context) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_inject_am(int rank, MPID_Comm * comm, int handler_id, const void * am_hdr, size_t am_hdr_sz, const void * data, MPI_Count count, MPI_Datatype datatype, void * shm_context) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_amv(int rank, MPID_Comm * comm, int handler_id, struct iovec * am_hdrs, size_t iov_len, const void * data, MPI_Count count, MPI_Datatype datatype, MPID_Request * sreq, void * shm_context) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_inject_amv(int rank, MPID_Comm * comm, int handler_id, struct iovec * am_hdrs, size_t iov_len, const void * data, MPI_Count count, MPI_Datatype datatype, void * shm_context) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_am_hdr_reply(MPIU_Context_id_t context_id, int src_rank, int handler_id, const void * am_hdr, size_t am_hdr_sz, MPID_Request * sreq) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_inject_am_hdr_reply(MPIU_Context_id_t context_id, int src_rank, int handler_id, const void * am_hdr, size_t am_hdr_sz) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_am_reply(MPIU_Context_id_t context_id, int src_rank, int handler_id, const void * am_hdr, size_t am_hdr_sz, const void * data, MPI_Count count, MPI_Datatype datatype, MPID_Request * sreq) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_inject_am_reply(MPIU_Context_id_t context_id, int src_rank, int handler_id, const void * am_hdr, size_t am_hdr_sz, const void * data, MPI_Count count, MPI_Datatype datatype) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_amv_reply(MPIU_Context_id_t context_id, int src_rank, int handler_id, struct iovec * am_hdr, size_t iov_len, const void * data, MPI_Count count, MPI_Datatype datatype, MPID_Request * sreq) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_inject_amv_reply(MPIU_Context_id_t context_id, int src_rank, int handler_id, struct iovec * am_hdrs, size_t iov_len, const void * data, MPI_Count count, MPI_Datatype datatype) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX size_t MPIDI_CH4_SHM_am_hdr_max_sz(void) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX size_t MPIDI_CH4_SHM_am_inject_max_sz(void) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_comm_get_lpid(MPID_Comm * comm_ptr, int idx, int * lpid_ptr, MPIU_BOOL is_remote) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_gpid_get(MPID_Comm * comm_ptr, int rank, MPIR_Gpid * gpid) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_get_node_id(MPID_Comm * comm, int rank, MPID_Node_id_t * id_p) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_get_max_node_id(MPID_Comm * comm, MPID_Node_id_t * max_id_p) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_getallincomm(MPID_Comm * comm_ptr, int local_size, MPIR_Gpid local_gpid[], int * singlePG) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_gpid_tolpidarray(int size, MPIR_Gpid gpid[], int lpid[]) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_create_intercomm_from_lpids(MPID_Comm * newcomm_ptr, int size, const int lpids[]) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_comm_create(MPID_Comm * comm) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_comm_destroy(MPID_Comm * comm) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX void MPIDI_CH4_SHM_am_request_init(MPID_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX void MPIDI_CH4_SHM_am_request_finalize(MPID_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ssend(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_startall(int count, MPID_Request *requests[]) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_init(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ssend_init(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_rsend_init(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_bsend_init(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_isend(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_issend(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_cancel_send(MPID_Request * sreq) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_recv_init(void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_recv(void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPI_Status * status, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_irecv(void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_imrecv(void * buf, int count, MPI_Datatype datatype, MPID_Request * message, MPID_Request ** rreqp) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_cancel_recv(MPID_Request * rreq) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX void * MPIDI_CH4_SHM_alloc_mem(size_t size, MPID_Info * info_ptr) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_free_mem(void *ptr) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_improbe(int source, int tag, MPID_Comm * comm, int context_offset, int *flag, MPID_Request ** message, MPI_Status * status) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iprobe(int source, int tag, MPID_Comm * comm, int context_offset, int * flag, MPI_Status * status) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_set_info(MPID_Win * win, MPID_Info * info) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_shared_query(MPID_Win * win, int rank, MPI_Aint * size, int * disp_unit, void * baseptr) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_put(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_start(MPID_Group * group, int assert, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_complete(MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_post(MPID_Group * group, int assert, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_wait(MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_test(MPID_Win * win, int * flag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_lock(int lock_type, int rank, int assert, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_unlock(int rank, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_get_info(MPID_Win * win, MPID_Info ** info_p_p) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_get(void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_free(MPID_Win ** win_ptr) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_fence(int assert, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_create(void * base, MPI_Aint length, int disp_unit, MPID_Info * info, MPID_Comm * comm_ptr, MPID_Win ** win_ptr) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_accumulate(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_attach(MPID_Win * win, void * base, MPI_Aint size) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_allocate_shared(MPI_Aint size, int disp_unit, MPID_Info * info_ptr, MPID_Comm * comm_ptr, void ** base_ptr, MPID_Win ** win_ptr) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_rput(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPID_Win * win, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_flush_local(int rank, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_detach(MPID_Win * win, const void * base) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_compare_and_swap(const void * origin_addr, const void * compare_addr, void * result_addr, MPI_Datatype datatype, int target_rank, MPI_Aint target_disp, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_raccumulate(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_rget_accumulate(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, void * result_addr, int result_count, MPI_Datatype result_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_fetch_and_op(const void * origin_addr, void * result_addr, MPI_Datatype datatype, int target_rank, MPI_Aint target_disp, MPI_Op op, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_allocate(MPI_Aint size, int disp_unit, MPID_Info * info, MPID_Comm * comm, void * baseptr, MPID_Win ** win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_flush(int rank, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_flush_local_all(MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_unlock_all(MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_create_dynamic(MPID_Info * info, MPID_Comm * comm, MPID_Win ** win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_rget(void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPID_Win * win, MPID_Request ** request) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_sync(MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_flush_all(MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_get_accumulate(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, void * result_addr, int result_count, MPI_Datatype result_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_lock_all(int assert, MPID_Win * win) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_barrier(MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_allreduce(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_allgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_scatter(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_scatterv(const void * sendbuf, const int * sendcounts, const int * displs, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_gather(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_gatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_alltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_alltoallv(const void * sendbuf, const int * sendcounts, const int * sdispls, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * rdispls, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_alltoallw(const void * sendbuf, const int * sendcounts, const int * sdispls, const MPI_Datatype sendtypes[], void * recvbuf, const int * recvcounts, const int * rdispls, const MPI_Datatype recvtypes[], MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_reduce(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_reduce_scatter(const void * sendbuf, void * recvbuf, const int * recvcounts, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_reduce_scatter_block(const void * sendbuf, void * recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_scan(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_exscan(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_neighbor_allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_neighbor_allgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_neighbor_alltoallv(const void * sendbuf, const int * sendcounts, const int * sdispls, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * rdispls, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_neighbor_alltoallw(const void * sendbuf, const int * sendcounts, const MPI_Aint * sdispls, const MPI_Datatype * sendtypes, void * recvbuf, const int * recvcounts, const MPI_Aint * rdispls, const MPI_Datatype * recvtypes, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_neighbor_alltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ineighbor_allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ineighbor_allgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ineighbor_alltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ineighbor_alltoallv(const void * sendbuf, const int * sendcounts, const int * sdispls, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * rdispls, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ineighbor_alltoallw(const void * sendbuf, const int * sendcounts, const MPI_Aint * sdispls, const MPI_Datatype * sendtypes, void * recvbuf, const int * recvcounts, const MPI_Aint * rdispls, const MPI_Datatype * recvtypes, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ibarrier(MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ibcast(void* buffer, int count, MPI_Datatype datatype, int root, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iallgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iallgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iallreduce(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ialltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ialltoallv(const void * sendbuf, const int * sendcounts, const int * sdispls, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * rdispls, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ialltoallw(const void * sendbuf, const int * sendcounts, const int * sdispls, const MPI_Datatype sendtypes[], void * recvbuf, const int * recvcounts, const int * rdispls, const MPI_Datatype recvtypes[], MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iexscan(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_igather(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_igatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ireduce_scatter_block(const void * sendbuf, void * recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ireduce_scatter(const void * sendbuf, void * recvbuf, const int * recvcounts, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ireduce(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPID_Comm * comm_ptr, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iscan(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iscatter(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;
MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iscatterv(const void * sendbuf, const int * sendcounts, const int * displs, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm_ptr, MPI_Request * req) MPIDI_CH4_SHM_STATIC_INLINE_SUFFIX;

#endif /* SHM_PROTOTYPES_H_INCLUDED */
