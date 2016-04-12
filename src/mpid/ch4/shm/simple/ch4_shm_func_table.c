/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#ifndef SHM_DIRECT
#define SHM_DISABLE_INLINES
#include <mpidimpl.h>
#include "ch4_shm_direct.h"
MPIDI_CH4_SHM_funcs_t MPIDI_CH4_SHM_simple_funcs = {
    MPIDI_CH4_SHM_init,
    MPIDI_CH4_SHM_finalize,
    MPIDI_CH4_SHM_progress,
    MPIDI_CH4_SHM_reg_hdr_handler,
    MPIDI_CH4_SHM_comm_connect,
    MPIDI_CH4_SHM_comm_disconnect,
    MPIDI_CH4_SHM_open_port,
    MPIDI_CH4_SHM_close_port,
    MPIDI_CH4_SHM_comm_accept,
    MPIDI_CH4_SHM_send_am_hdr,
    MPIDI_CH4_SHM_inject_am_hdr,
    MPIDI_CH4_SHM_send_am,
    MPIDI_CH4_SHM_inject_am,
    MPIDI_CH4_SHM_send_amv,
    MPIDI_CH4_SHM_inject_amv,
    MPIDI_CH4_SHM_send_am_hdr_reply,
    MPIDI_CH4_SHM_inject_am_hdr_reply,
    MPIDI_CH4_SHM_send_am_reply,
    MPIDI_CH4_SHM_inject_am_reply,
    MPIDI_CH4_SHM_send_amv_reply,
    MPIDI_CH4_SHM_inject_amv_reply,
    MPIDI_CH4_SHM_am_hdr_max_sz,
    MPIDI_CH4_SHM_am_inject_max_sz,
    MPIDI_CH4_SHM_comm_get_lpid,
    MPIDI_CH4_SHM_gpid_get,
    MPIDI_CH4_SHM_get_node_id,
    MPIDI_CH4_SHM_get_max_node_id,
    MPIDI_CH4_SHM_getallincomm,
    MPIDI_CH4_SHM_gpid_tolpidarray,
    MPIDI_CH4_SHM_create_intercomm_from_lpids,
    MPIDI_CH4_SHM_comm_create,
    MPIDI_CH4_SHM_comm_destroy,
    MPIDI_CH4_SHM_am_request_init,
};

MPIDI_CH4_SHM_native_funcs_t MPIDI_CH4_SHM_native_simple_funcs = {
    MPIDI_CH4_SHM_send,
    MPIDI_CH4_SHM_ssend,
    MPIDI_CH4_SHM_startall,
    MPIDI_CH4_SHM_send_init,
    MPIDI_CH4_SHM_ssend_init,
    MPIDI_CH4_SHM_rsend_init,
    MPIDI_CH4_SHM_bsend_init,
    MPIDI_CH4_SHM_isend,
    MPIDI_CH4_SHM_issend,
    MPIDI_CH4_SHM_cancel_send,
    MPIDI_CH4_SHM_recv_init,
    MPIDI_CH4_SHM_recv,
    MPIDI_CH4_SHM_irecv,
    MPIDI_CH4_SHM_imrecv,
    MPIDI_CH4_SHM_cancel_recv,
    MPIDI_CH4_SHM_alloc_mem,
    MPIDI_CH4_SHM_free_mem,
    MPIDI_CH4_SHM_improbe,
    MPIDI_CH4_SHM_iprobe,
    MPIDI_CH4_SHM_win_set_info,
    MPIDI_CH4_SHM_win_shared_query,
    MPIDI_CH4_SHM_put,
    MPIDI_CH4_SHM_win_start,
    MPIDI_CH4_SHM_win_complete,
    MPIDI_CH4_SHM_win_post,
    MPIDI_CH4_SHM_win_wait,
    MPIDI_CH4_SHM_win_test,
    MPIDI_CH4_SHM_win_lock,
    MPIDI_CH4_SHM_win_unlock,
    MPIDI_CH4_SHM_win_get_info,
    MPIDI_CH4_SHM_get,
    MPIDI_CH4_SHM_win_free,
    MPIDI_CH4_SHM_win_fence,
    MPIDI_CH4_SHM_win_create,
    MPIDI_CH4_SHM_accumulate,
    MPIDI_CH4_SHM_win_attach,
    MPIDI_CH4_SHM_win_allocate_shared,
    MPIDI_CH4_SHM_rput,
    MPIDI_CH4_SHM_win_flush_local,
    MPIDI_CH4_SHM_win_detach,
    MPIDI_CH4_SHM_compare_and_swap,
    MPIDI_CH4_SHM_raccumulate,
    MPIDI_CH4_SHM_rget_accumulate,
    MPIDI_CH4_SHM_fetch_and_op,
    MPIDI_CH4_SHM_win_allocate,
    MPIDI_CH4_SHM_win_flush,
    MPIDI_CH4_SHM_win_flush_local_all,
    MPIDI_CH4_SHM_win_unlock_all,
    MPIDI_CH4_SHM_win_create_dynamic,
    MPIDI_CH4_SHM_rget,
    MPIDI_CH4_SHM_win_sync,
    MPIDI_CH4_SHM_win_flush_all,
    MPIDI_CH4_SHM_get_accumulate,
    MPIDI_CH4_SHM_win_lock_all,
    MPIDI_CH4_SHM_barrier,
    MPIDI_CH4_SHM_bcast,
    MPIDI_CH4_SHM_allreduce,
    MPIDI_CH4_SHM_allgather,
    MPIDI_CH4_SHM_allgatherv,
    MPIDI_CH4_SHM_scatter,
    MPIDI_CH4_SHM_scatterv,
    MPIDI_CH4_SHM_gather,
    MPIDI_CH4_SHM_gatherv,
    MPIDI_CH4_SHM_alltoall,
    MPIDI_CH4_SHM_alltoallv,
    MPIDI_CH4_SHM_alltoallw,
    MPIDI_CH4_SHM_reduce,
    MPIDI_CH4_SHM_reduce_scatter,
    MPIDI_CH4_SHM_reduce_scatter_block,
    MPIDI_CH4_SHM_scan,
    MPIDI_CH4_SHM_exscan,
    MPIDI_CH4_SHM_neighbor_allgather,
    MPIDI_CH4_SHM_neighbor_allgatherv,
    MPIDI_CH4_SHM_neighbor_alltoall,
    MPIDI_CH4_SHM_neighbor_alltoallv,
    MPIDI_CH4_SHM_neighbor_alltoallw,
    MPIDI_CH4_SHM_ineighbor_allgather,
    MPIDI_CH4_SHM_ineighbor_allgatherv,
    MPIDI_CH4_SHM_ineighbor_alltoall,
    MPIDI_CH4_SHM_ineighbor_alltoallv,
    MPIDI_CH4_SHM_ineighbor_alltoallw,
    MPIDI_CH4_SHM_ibarrier,
    MPIDI_CH4_SHM_ibcast,
    MPIDI_CH4_SHM_iallgather,
    MPIDI_CH4_SHM_iallgatherv,
    MPIDI_CH4_SHM_iallreduce,
    MPIDI_CH4_SHM_ialltoall,
    MPIDI_CH4_SHM_ialltoallv,
    MPIDI_CH4_SHM_ialltoallw,
    MPIDI_CH4_SHM_iexscan,
    MPIDI_CH4_SHM_igather,
    MPIDI_CH4_SHM_igatherv,
    MPIDI_CH4_SHM_ireduce_scatter_block,
    MPIDI_CH4_SHM_ireduce_scatter,
    MPIDI_CH4_SHM_ireduce,
    MPIDI_CH4_SHM_iscan,
    MPIDI_CH4_SHM_iscatter,
    MPIDI_CH4_SHM_iscatterv,
};
#endif
