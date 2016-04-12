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
#ifndef SHM_IMPL_PROTOTYPES_H_INCLUDED
#define SHM_IMPL_PROTOTYPES_H_INCLUDED

#ifndef SHM_DIRECT
#ifndef SHM_DISABLE_INLINES

#ifndef MPIDI_CH4_SHM_STATIC_INLINE_PREFIX
#define MPIDI_CH4_SHM_STATIC_INLINE_PREFIX __attribute__((always_inline)) static inline
#endif

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_init(int rank, int size) {
    return MPIDI_CH4_SHM_func->init( rank, size);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_finalize(void) {
    return MPIDI_CH4_SHM_func->finalize();
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_progress(int blocking) {
    return MPIDI_CH4_SHM_func->progress( blocking);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_reg_hdr_handler(int handler_id, MPIDI_CH4_SHM_am_origin_handler_fn origin_handler_fn, MPIDI_CH4_SHM_am_target_handler_fn target_handler_fn) {
    return MPIDI_CH4_SHM_func->reg_hdr_handler( handler_id, origin_handler_fn, target_handler_fn);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_comm_connect(const char * port_name, MPID_Info * info, int root, MPID_Comm * comm, MPID_Comm ** newcomm_ptr) {
    return MPIDI_CH4_SHM_func->comm_connect( port_name, info, root, comm, newcomm_ptr);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_comm_disconnect(MPID_Comm * comm_ptr) {
    return MPIDI_CH4_SHM_func->comm_disconnect( comm_ptr);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_open_port(MPID_Info * info_ptr, char * port_name) {
    return MPIDI_CH4_SHM_func->open_port( info_ptr, port_name);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_close_port(const char * port_name) {
    return MPIDI_CH4_SHM_func->close_port( port_name);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_comm_accept(const char * port_name, MPID_Info * info, int root, MPID_Comm * comm, MPID_Comm ** newcomm_ptr) {
    return MPIDI_CH4_SHM_func->comm_accept( port_name, info, root, comm, newcomm_ptr);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_am_hdr(int rank, MPID_Comm * comm, int handler_id, const void* am_hdr, size_t am_hdr_sz, MPID_Request * sreq, void * shm_context) {
    return MPIDI_CH4_SHM_func->send_am_hdr( rank, comm, handler_id, am_hdr, am_hdr_sz, sreq, shm_context);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_inject_am_hdr(int rank, MPID_Comm * comm, int handler_id, const void* am_hdr, size_t am_hdr_sz, void * shm_context) {
    return MPIDI_CH4_SHM_func->inject_am_hdr( rank, comm, handler_id, am_hdr, am_hdr_sz, shm_context);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_am(int rank, MPID_Comm * comm, int handler_id, const void * am_hdr, size_t am_hdr_sz, const void * data, MPI_Count count, MPI_Datatype datatype, MPID_Request * sreq, void * shm_context) {
    return MPIDI_CH4_SHM_func->send_am( rank, comm, handler_id, am_hdr, am_hdr_sz, data, count, datatype, sreq, shm_context);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_inject_am(int rank, MPID_Comm * comm, int handler_id, const void * am_hdr, size_t am_hdr_sz, const void * data, MPI_Count count, MPI_Datatype datatype, void * shm_context) {
    return MPIDI_CH4_SHM_func->inject_am( rank, comm, handler_id, am_hdr, am_hdr_sz, data, count, datatype, shm_context);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_amv(int rank, MPID_Comm * comm, int handler_id, struct iovec * am_hdrs, size_t iov_len, const void * data, MPI_Count count, MPI_Datatype datatype, MPID_Request * sreq, void * shm_context) {
    return MPIDI_CH4_SHM_func->send_amv( rank, comm, handler_id, am_hdrs, iov_len, data, count, datatype, sreq, shm_context);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_inject_amv(int rank, MPID_Comm * comm, int handler_id, struct iovec * am_hdrs, size_t iov_len, const void * data, MPI_Count count, MPI_Datatype datatype, void * shm_context) {
    return MPIDI_CH4_SHM_func->inject_amv( rank, comm, handler_id, am_hdrs, iov_len, data, count, datatype, shm_context);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_am_hdr_reply(void * reply_token, int handler_id, const void * am_hdr, size_t am_hdr_sz, MPID_Request * sreq) {
    return MPIDI_CH4_SHM_func->send_am_hdr_reply( reply_token, handler_id, am_hdr, am_hdr_sz, sreq);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_inject_am_hdr_reply(void * reply_token, int handler_id, const void * am_hdr, size_t am_hdr_sz) {
    return MPIDI_CH4_SHM_func->inject_am_hdr_reply( reply_token, handler_id, am_hdr, am_hdr_sz);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_am_reply(void * reply_token, int handler_id, const void * am_hdr, size_t am_hdr_sz, const void * data, MPI_Count count, MPI_Datatype datatype, MPID_Request * sreq) {
    return MPIDI_CH4_SHM_func->send_am_reply( reply_token, handler_id, am_hdr, am_hdr_sz, data, count, datatype, sreq);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_inject_am_reply(void * reply_token, int handler_id, const void * am_hdr, size_t am_hdr_sz, const void * data, MPI_Count count, MPI_Datatype datatype) {
    return MPIDI_CH4_SHM_func->inject_am_reply( reply_token, handler_id, am_hdr, am_hdr_sz, data, count, datatype);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_amv_reply(void * reply_token, int handler_id, struct iovec * am_hdr, size_t iov_len, const void * data, MPI_Count count, MPI_Datatype datatype, MPID_Request * sreq) {
    return MPIDI_CH4_SHM_func->send_amv_reply( reply_token, handler_id, am_hdr, iov_len, data, count, datatype, sreq);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_inject_amv_reply(void * reply_token, int handler_id, struct iovec * am_hdrs, size_t iov_len, const void * data, MPI_Count count, MPI_Datatype datatype) {
    return MPIDI_CH4_SHM_func->inject_amv_reply( reply_token, handler_id, am_hdrs, iov_len, data, count, datatype);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX size_t MPIDI_CH4_SHM_am_hdr_max_sz(void) {
    return MPIDI_CH4_SHM_func->am_hdr_max_sz();
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX size_t MPIDI_CH4_SHM_am_inject_max_sz(void) {
    return MPIDI_CH4_SHM_func->am_inject_max_sz();
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_comm_get_lpid(MPID_Comm * comm_ptr, int idx, int * lpid_ptr, MPIU_BOOL is_remote) {
    return MPIDI_CH4_SHM_func->comm_get_lpid( comm_ptr, idx, lpid_ptr, is_remote);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_gpid_get(MPID_Comm * comm_ptr, int rank, MPIR_Gpid * gpid) {
    return MPIDI_CH4_SHM_func->gpid_get( comm_ptr, rank, gpid);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_get_node_id(MPID_Comm * comm, int rank, MPID_Node_id_t * id_p) {
    return MPIDI_CH4_SHM_func->get_node_id( comm, rank, id_p);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_get_max_node_id(MPID_Comm * comm, MPID_Node_id_t * max_id_p) {
    return MPIDI_CH4_SHM_func->get_max_node_id( comm, max_id_p);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_getallincomm(MPID_Comm * comm_ptr, int local_size, MPIR_Gpid local_gpid[], int * singlePG) {
    return MPIDI_CH4_SHM_func->getallincomm( comm_ptr, local_size, local_gpid, singlePG);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_gpid_tolpidarray(int size, MPIR_Gpid gpid[], int lpid[]) {
    return MPIDI_CH4_SHM_func->gpid_tolpidarray( size, gpid, lpid);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_create_intercomm_from_lpids(MPID_Comm * newcomm_ptr, int size, const int lpids[]) {
    return MPIDI_CH4_SHM_func->create_intercomm_from_lpids( newcomm_ptr, size, lpids);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_comm_create(MPID_Comm * comm) {
    return MPIDI_CH4_SHM_func->comm_create( comm);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_comm_destroy(MPID_Comm * comm) {
    return MPIDI_CH4_SHM_func->comm_destroy( comm);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX void MPIDI_CH4_SHM_am_request_init(MPID_Request * req) {
    return MPIDI_CH4_SHM_func->am_request_init( req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX void MPIDI_CH4_SHM_am_request_finalize(MPID_Request * req) {
    return MPIDI_CH4_SHM_func->am_request_finalize( req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->send( buf, count, datatype, rank, tag, comm, context_offset, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ssend(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->ssend( buf, count, datatype, rank, tag, comm, context_offset, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_startall(int count, MPID_Request requests[]) {
    return MPIDI_CH4_SHM_native_func->startall( count, requests);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_send_init(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->send_init( buf, count, datatype, rank, tag, comm, context_offset, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ssend_init(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->ssend_init( buf, count, datatype, rank, tag, comm, context_offset, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_rsend_init(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->rsend_init( buf, count, datatype, rank, tag, comm, context_offset, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_bsend_init(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->bsend_init( buf, count, datatype, rank, tag, comm, context_offset, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_isend(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->isend( buf, count, datatype, rank, tag, comm, context_offset, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_issend(const void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->issend( buf, count, datatype, rank, tag, comm, context_offset, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_cancel_send(MPID_Request * sreq) {
    return MPIDI_CH4_SHM_native_func->cancel_send( sreq);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_recv_init(void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->recv_init( buf, count, datatype, rank, tag, comm, context_offset, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_recv(void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPI_Status * status, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->recv( buf, count, datatype, rank, tag, comm, context_offset, status, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_irecv(void * buf, int count, MPI_Datatype datatype, int rank, int tag, MPID_Comm * comm, int context_offset, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->irecv( buf, count, datatype, rank, tag, comm, context_offset, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_imrecv(void * buf, int count, MPI_Datatype datatype, MPID_Request * message, MPID_Request ** rreqp) {
    return MPIDI_CH4_SHM_native_func->imrecv( buf, count, datatype, message, rreqp);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_cancel_recv(MPID_Request * rreq) {
    return MPIDI_CH4_SHM_native_func->cancel_recv( rreq);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX void * MPIDI_CH4_SHM_alloc_mem(size_t size, MPID_Info * info_ptr) {
    return MPIDI_CH4_SHM_native_func->alloc_mem( size, info_ptr);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_free_mem(void *ptr) {
    return MPIDI_CH4_SHM_native_func->free_mem(ptr);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_improbe(int source, int tag, MPID_Comm * comm, int context_offset, int *flag, MPID_Request ** message, MPI_Status * status) {
    return MPIDI_CH4_SHM_native_func->improbe( source, tag, comm, context_offset, flag, message, status);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iprobe(int source, int tag, MPID_Comm * comm, int context_offset, int * flag, MPI_Status * status) {
    return MPIDI_CH4_SHM_native_func->iprobe( source, tag, comm, context_offset, flag, status);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_set_info(MPID_Win * win, MPID_Info * info) {
    return MPIDI_CH4_SHM_native_func->win_set_info( win, info);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_shared_query(MPID_Win * win, int rank, MPI_Aint * size, int * disp_unit, void * baseptr) {
    return MPIDI_CH4_SHM_native_func->win_shared_query( win, rank, size, disp_unit, baseptr);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_put(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->put( origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_start(MPID_Group * group, int assert, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_start( group, assert, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_complete(MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_complete( win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_post(MPID_Group * group, int assert, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_post( group, assert, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_wait(MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_wait( win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_test(MPID_Win * win, int * flag) {
    return MPIDI_CH4_SHM_native_func->win_test( win, flag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_lock(int lock_type, int rank, int assert, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_lock( lock_type, rank, assert, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_unlock(int rank, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_unlock( rank, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_get_info(MPID_Win * win, MPID_Info ** info_p_p) {
    return MPIDI_CH4_SHM_native_func->win_get_info( win, info_p_p);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_get(void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->get( origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_free(MPID_Win ** win_ptr) {
    return MPIDI_CH4_SHM_native_func->win_free( win_ptr);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_fence(int assert, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_fence( assert, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_create(void * base, MPI_Aint length, int disp_unit, MPID_Info * info, MPID_Comm * comm_ptr, MPID_Win ** win_ptr) {
    return MPIDI_CH4_SHM_native_func->win_create( base, length, disp_unit, info, comm_ptr, win_ptr);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_accumulate(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->accumulate( origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_attach(MPID_Win * win, void * base, MPI_Aint size) {
    return MPIDI_CH4_SHM_native_func->win_attach( win, base, size);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_allocate_shared(MPI_Aint size, int disp_unit, MPID_Info * info_ptr, MPID_Comm * comm_ptr, void ** base_ptr, MPID_Win ** win_ptr) {
    return MPIDI_CH4_SHM_native_func->win_allocate_shared( size, disp_unit, info_ptr, comm_ptr, base_ptr, win_ptr);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_rput(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPID_Win * win, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->rput( origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_flush_local(int rank, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_flush_local( rank, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_detach(MPID_Win * win, const void * base) {
    return MPIDI_CH4_SHM_native_func->win_detach( win, base);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_compare_and_swap(const void * origin_addr, const void * compare_addr, void * result_addr, MPI_Datatype datatype, int target_rank, MPI_Aint target_disp, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->compare_and_swap( origin_addr, compare_addr, result_addr, datatype, target_rank, target_disp, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_raccumulate(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->raccumulate( origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_rget_accumulate(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, void * result_addr, int result_count, MPI_Datatype result_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->rget_accumulate( origin_addr, origin_count, origin_datatype, result_addr, result_count, result_datatype, target_rank, target_disp, target_count, target_datatype, op, win, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_fetch_and_op(const void * origin_addr, void * result_addr, MPI_Datatype datatype, int target_rank, MPI_Aint target_disp, MPI_Op op, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->fetch_and_op( origin_addr, result_addr, datatype, target_rank, target_disp, op, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_allocate(MPI_Aint size, int disp_unit, MPID_Info * info, MPID_Comm * comm, void * baseptr, MPID_Win ** win) {
    return MPIDI_CH4_SHM_native_func->win_allocate( size, disp_unit, info, comm, baseptr, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_flush(int rank, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_flush( rank, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_flush_local_all(MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_flush_local_all( win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_unlock_all(MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_unlock_all( win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_create_dynamic(MPID_Info * info, MPID_Comm * comm, MPID_Win ** win) {
    return MPIDI_CH4_SHM_native_func->win_create_dynamic( info, comm, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_rget(void * origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPID_Win * win, MPID_Request ** request) {
    return MPIDI_CH4_SHM_native_func->rget( origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win, request);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_sync(MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_sync( win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_flush_all(MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_flush_all( win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_get_accumulate(const void * origin_addr, int origin_count, MPI_Datatype origin_datatype, void * result_addr, int result_count, MPI_Datatype result_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, MPI_Op op, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->get_accumulate( origin_addr, origin_count, origin_datatype, result_addr, result_count, result_datatype, target_rank, target_disp, target_count, target_datatype, op, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_win_lock_all(int assert, MPID_Win * win) {
    return MPIDI_CH4_SHM_native_func->win_lock_all( assert, win);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_barrier(MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->barrier( comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->bcast( buffer, count, datatype, root, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_allreduce(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->allreduce( sendbuf, recvbuf, count, datatype, op, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->allgather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_allgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->allgatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_scatter(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->scatter( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_scatterv(const void * sendbuf, const int * sendcounts, const int * displs, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->scatterv( sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm_ptr, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_gather(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->gather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_gatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->gatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_alltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->alltoall( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_alltoallv(const void * sendbuf, const int * sendcounts, const int * sdispls, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * rdispls, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->alltoallv( sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_alltoallw(const void * sendbuf, const int * sendcounts, const int * sdispls, const MPI_Datatype sendtypes[], void * recvbuf, const int * recvcounts, const int * rdispls, const MPI_Datatype recvtypes[], MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->alltoallw( sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_reduce(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->reduce( sendbuf, recvbuf, count, datatype, op, root, comm_ptr, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_reduce_scatter(const void * sendbuf, void * recvbuf, const int * recvcounts, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->reduce_scatter( sendbuf, recvbuf, recvcounts, datatype, op, comm_ptr, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_reduce_scatter_block(const void * sendbuf, void * recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->reduce_scatter_block( sendbuf, recvbuf, recvcount, datatype, op, comm_ptr, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_scan(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->scan( sendbuf, recvbuf, count, datatype, op, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_exscan(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->exscan( sendbuf, recvbuf, count, datatype, op, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_neighbor_allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->neighbor_allgather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_neighbor_allgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->neighbor_allgatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_neighbor_alltoallv(const void * sendbuf, const int * sendcounts, const int * sdispls, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * rdispls, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->neighbor_alltoallv( sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_neighbor_alltoallw(const void * sendbuf, const int * sendcounts, const MPI_Aint * sdispls, const MPI_Datatype * sendtypes, void * recvbuf, const int * recvcounts, const MPI_Aint * rdispls, const MPI_Datatype * recvtypes, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->neighbor_alltoallw( sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_neighbor_alltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPIR_Errflag_t * errflag) {
    return MPIDI_CH4_SHM_native_func->neighbor_alltoall( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, errflag);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ineighbor_allgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ineighbor_allgather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ineighbor_allgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ineighbor_allgatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ineighbor_alltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ineighbor_alltoall( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ineighbor_alltoallv(const void * sendbuf, const int * sendcounts, const int * sdispls, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * rdispls, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ineighbor_alltoallv( sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ineighbor_alltoallw(const void * sendbuf, const int * sendcounts, const MPI_Aint * sdispls, const MPI_Datatype * sendtypes, void * recvbuf, const int * recvcounts, const MPI_Aint * rdispls, const MPI_Datatype * recvtypes, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ineighbor_alltoallw( sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ibarrier(MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ibarrier( comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ibcast(void* buffer, int count, MPI_Datatype datatype, int root, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ibcast( buffer, count, datatype, root, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iallgather(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->iallgather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iallgatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->iallgatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iallreduce(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->iallreduce( sendbuf, recvbuf, count, datatype, op, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ialltoall(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ialltoall( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ialltoallv(const void * sendbuf, const int * sendcounts, const int * sdispls, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * rdispls, MPI_Datatype recvtype, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ialltoallv( sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ialltoallw(const void * sendbuf, const int * sendcounts, const int * sdispls, const MPI_Datatype sendtypes[], void * recvbuf, const int * recvcounts, const int * rdispls, const MPI_Datatype recvtypes[], MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ialltoallw( sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iexscan(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->iexscan( sendbuf, recvbuf, count, datatype, op, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_igather(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->igather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_igatherv(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, const int * recvcounts, const int * displs, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->igatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ireduce_scatter_block(const void * sendbuf, void * recvbuf, int recvcount, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ireduce_scatter_block( sendbuf, recvbuf, recvcount, datatype, op, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ireduce_scatter(const void * sendbuf, void * recvbuf, const int * recvcounts, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ireduce_scatter( sendbuf, recvbuf, recvcounts, datatype, op, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_ireduce(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPID_Comm * comm_ptr, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->ireduce( sendbuf, recvbuf, count, datatype, op, root, comm_ptr, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iscan(const void * sendbuf, void * recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->iscan( sendbuf, recvbuf, count, datatype, op, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iscatter(const void * sendbuf, int sendcount, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->iscatter( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm, req);
};

MPIDI_CH4_SHM_STATIC_INLINE_PREFIX int MPIDI_CH4_SHM_iscatterv(const void * sendbuf, const int * sendcounts, const int * displs, MPI_Datatype sendtype, void * recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPID_Comm * comm_ptr, MPI_Request * req) {
    return MPIDI_CH4_SHM_native_func->iscatterv( sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm_ptr, req);
};

#endif /* SHM_DISABLE_INLINES  */

#else

#define __shm_direct_stub__     0
#define __shm_direct_simple__   1

#if SHM_DIRECT==__shm_direct_stub__
#include "../stub/ch4_shm_direct.h"
#elif SHM_DIRECT==__shm_direct_simple__
#include "../simple/ch4_shm_direct.h"
#else
#error "No direct shm included"
#endif


#endif /* SHM_DIRECT           */

#endif
