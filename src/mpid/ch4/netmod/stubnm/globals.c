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

#ifndef NETMOD_DIRECT
#define NETMOD_DISABLE_INLINES
#include <mpidimpl.h>
#include "netmod_direct.h"
MPIDI_NM_funcs_t MPIDI_NM_stubnm_funcs = {
    MPIDI_NM_init,
    MPIDI_NM_finalize,
    MPIDI_NM_progress,
    MPIDI_NM_comm_connect,
    MPIDI_NM_comm_disconnect,
    MPIDI_NM_open_port,
    MPIDI_NM_close_port,
    MPIDI_NM_comm_accept,
    MPIDI_NM_comm_get_lpid,
    MPIDI_NM_gpid_get,
    MPIDI_NM_get_node_id,
    MPIDI_NM_get_max_node_id,
    MPIDI_NM_getallincomm,
    MPIDI_NM_gpid_tolpidarray,
    MPIDI_NM_create_intercomm_from_lpids,
    MPIDI_NM_comm_create,
    MPIDI_NM_comm_destroy,
    MPIDI_NM_am_request_init,
    MPIDI_NM_am_request_finalize,
    MPIDI_NM_reg_hdr_handler,
    MPIDI_NM_send_am_hdr,
    MPIDI_NM_inject_am_hdr,
    MPIDI_NM_send_am,
    MPIDI_NM_send_amv,
    MPIDI_NM_send_amv_hdr,
    MPIDI_NM_send_am_hdr_reply,
    MPIDI_NM_inject_am_hdr_reply,
    MPIDI_NM_send_am_reply,
    MPIDI_NM_send_amv_reply,
    MPIDI_NM_am_hdr_max_sz,
    MPIDI_NM_am_inject_max_sz
};

MPIDI_NM_native_funcs_t MPIDI_NM_native_stubnm_funcs = {
    MPIDI_NM_send,
    MPIDI_NM_ssend,
    MPIDI_NM_startall,
    MPIDI_NM_send_init,
    MPIDI_NM_ssend_init,
    MPIDI_NM_rsend_init,
    MPIDI_NM_bsend_init,
    MPIDI_NM_isend,
    MPIDI_NM_issend,
    MPIDI_NM_cancel_send,
    MPIDI_NM_recv_init,
    MPIDI_NM_recv,
    MPIDI_NM_irecv,
    MPIDI_NM_imrecv,
    MPIDI_NM_cancel_recv,
    MPIDI_NM_alloc_mem,
    MPIDI_NM_free_mem,
    MPIDI_NM_improbe,
    MPIDI_NM_iprobe,
    MPIDI_NM_win_set_info,
    MPIDI_NM_win_shared_query,
    MPIDI_NM_put,
    MPIDI_NM_win_start,
    MPIDI_NM_win_complete,
    MPIDI_NM_win_post,
    MPIDI_NM_win_wait,
    MPIDI_NM_win_test,
    MPIDI_NM_win_lock,
    MPIDI_NM_win_unlock,
    MPIDI_NM_win_get_info,
    MPIDI_NM_get,
    MPIDI_NM_win_free,
    MPIDI_NM_win_fence,
    MPIDI_NM_win_create,
    MPIDI_NM_accumulate,
    MPIDI_NM_win_attach,
    MPIDI_NM_win_allocate_shared,
    MPIDI_NM_rput,
    MPIDI_NM_win_flush_local,
    MPIDI_NM_win_detach,
    MPIDI_NM_compare_and_swap,
    MPIDI_NM_raccumulate,
    MPIDI_NM_rget_accumulate,
    MPIDI_NM_fetch_and_op,
    MPIDI_NM_win_allocate,
    MPIDI_NM_win_flush,
    MPIDI_NM_win_flush_local_all,
    MPIDI_NM_win_unlock_all,
    MPIDI_NM_win_create_dynamic,
    MPIDI_NM_rget,
    MPIDI_NM_win_sync,
    MPIDI_NM_win_flush_all,
    MPIDI_NM_get_accumulate,
    MPIDI_NM_win_lock_all,
    MPIDI_NM_rank_is_local,
    MPIDI_NM_barrier,
    MPIDI_NM_bcast,
    MPIDI_NM_allreduce,
    MPIDI_NM_allgather,
    MPIDI_NM_allgatherv,
    MPIDI_NM_scatter,
    MPIDI_NM_scatterv,
    MPIDI_NM_gather,
    MPIDI_NM_gatherv,
    MPIDI_NM_alltoall,
    MPIDI_NM_alltoallv,
    MPIDI_NM_alltoallw,
    MPIDI_NM_reduce,
    MPIDI_NM_reduce_scatter,
    MPIDI_NM_reduce_scatter_block,
    MPIDI_NM_scan,
    MPIDI_NM_exscan,
    MPIDI_NM_neighbor_allgather,
    MPIDI_NM_neighbor_allgatherv,
    MPIDI_NM_neighbor_alltoall,
    MPIDI_NM_neighbor_alltoallv,
    MPIDI_NM_neighbor_alltoallw,
    MPIDI_NM_ineighbor_allgather,
    MPIDI_NM_ineighbor_allgatherv,
    MPIDI_NM_ineighbor_alltoall,
    MPIDI_NM_ineighbor_alltoallv,
    MPIDI_NM_ineighbor_alltoallw,
    MPIDI_NM_ibarrier,
    MPIDI_NM_ibcast,
    MPIDI_NM_iallgather,
    MPIDI_NM_iallgatherv,
    MPIDI_NM_iallreduce,
    MPIDI_NM_ialltoall,
    MPIDI_NM_ialltoallv,
    MPIDI_NM_ialltoallw,
    MPIDI_NM_iexscan,
    MPIDI_NM_igather,
    MPIDI_NM_igatherv,
    MPIDI_NM_ireduce_scatter_block,
    MPIDI_NM_ireduce_scatter,
    MPIDI_NM_ireduce,
    MPIDI_NM_iscan,
    MPIDI_NM_iscatter,
    MPIDI_NM_iscatterv,
    MPIDI_NM_datatype_commit,
    MPIDI_NM_datatype_dup,
    MPIDI_NM_datatype_destroy,
    MPIDI_NM_op_commit,
    MPIDI_NM_op_destroy,
};
#endif
