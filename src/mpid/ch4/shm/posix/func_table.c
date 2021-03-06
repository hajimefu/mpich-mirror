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
#include "shm_direct.h"
MPIDI_SHM_funcs_t MPIDI_SHM_posix_funcs = {
    MPIDI_SHM_init,
    MPIDI_SHM_finalize,
    MPIDI_SHM_progress,
    MPIDI_SHM_reg_hdr_handler,
    MPIDI_SHM_comm_connect,
    MPIDI_SHM_comm_disconnect,
    MPIDI_SHM_open_port,
    MPIDI_SHM_close_port,
    MPIDI_SHM_comm_accept,
    MPIDI_SHM_send_am_hdr,
    MPIDI_SHM_inject_am_hdr,
    MPIDI_SHM_send_am,
    MPIDI_SHM_inject_am,
    MPIDI_SHM_send_amv,
    MPIDI_SHM_inject_amv,
    MPIDI_SHM_send_am_hdr_reply,
    MPIDI_SHM_inject_am_hdr_reply,
    MPIDI_SHM_send_am_reply,
    MPIDI_SHM_inject_am_reply,
    MPIDI_SHM_send_amv_reply,
    MPIDI_SHM_inject_amv_reply,
    MPIDI_SHM_am_hdr_max_sz,
    MPIDI_SHM_am_inject_max_sz,
    MPIDI_SHM_comm_get_lpid,
    MPIDI_SHM_gpid_get,
    MPIDI_SHM_get_node_id,
    MPIDI_SHM_get_max_node_id,
    MPIDI_SHM_getallincomm,
    MPIDI_SHM_gpid_tolpidarray,
    MPIDI_SHM_create_intercomm_from_lpids,
    MPIDI_SHM_comm_create,
    MPIDI_SHM_comm_destroy,
    MPIDI_SHM_am_request_init,
};

MPIDI_SHM_native_funcs_t MPIDI_SHM_native_posix_funcs = {
    MPIDI_SHM_send,
    MPIDI_SHM_ssend,
    MPIDI_SHM_startall,
    MPIDI_SHM_send_init,
    MPIDI_SHM_ssend_init,
    MPIDI_SHM_rsend_init,
    MPIDI_SHM_bsend_init,
    MPIDI_SHM_isend,
    MPIDI_SHM_issend,
    MPIDI_SHM_cancel_send,
    MPIDI_SHM_recv_init,
    MPIDI_SHM_recv,
    MPIDI_SHM_irecv,
    MPIDI_SHM_imrecv,
    MPIDI_SHM_cancel_recv,
    MPIDI_SHM_alloc_mem,
    MPIDI_SHM_free_mem,
    MPIDI_SHM_improbe,
    MPIDI_SHM_iprobe,
    MPIDI_SHM_win_set_info,
    MPIDI_SHM_win_shared_query,
    MPIDI_SHM_put,
    MPIDI_SHM_win_start,
    MPIDI_SHM_win_complete,
    MPIDI_SHM_win_post,
    MPIDI_SHM_win_wait,
    MPIDI_SHM_win_test,
    MPIDI_SHM_win_lock,
    MPIDI_SHM_win_unlock,
    MPIDI_SHM_win_get_info,
    MPIDI_SHM_get,
    MPIDI_SHM_win_free,
    MPIDI_SHM_win_fence,
    MPIDI_SHM_win_create,
    MPIDI_SHM_accumulate,
    MPIDI_SHM_win_attach,
    MPIDI_SHM_win_allocate_shared,
    MPIDI_SHM_rput,
    MPIDI_SHM_win_flush_local,
    MPIDI_SHM_win_detach,
    MPIDI_SHM_compare_and_swap,
    MPIDI_SHM_raccumulate,
    MPIDI_SHM_rget_accumulate,
    MPIDI_SHM_fetch_and_op,
    MPIDI_SHM_win_allocate,
    MPIDI_SHM_win_flush,
    MPIDI_SHM_win_flush_local_all,
    MPIDI_SHM_win_unlock_all,
    MPIDI_SHM_win_create_dynamic,
    MPIDI_SHM_rget,
    MPIDI_SHM_win_sync,
    MPIDI_SHM_win_flush_all,
    MPIDI_SHM_get_accumulate,
    MPIDI_SHM_win_lock_all,
    MPIDI_SHM_barrier,
    MPIDI_SHM_bcast,
    MPIDI_SHM_allreduce,
    MPIDI_SHM_allgather,
    MPIDI_SHM_allgatherv,
    MPIDI_SHM_scatter,
    MPIDI_SHM_scatterv,
    MPIDI_SHM_gather,
    MPIDI_SHM_gatherv,
    MPIDI_SHM_alltoall,
    MPIDI_SHM_alltoallv,
    MPIDI_SHM_alltoallw,
    MPIDI_SHM_reduce,
    MPIDI_SHM_reduce_scatter,
    MPIDI_SHM_reduce_scatter_block,
    MPIDI_SHM_scan,
    MPIDI_SHM_exscan,
    MPIDI_SHM_neighbor_allgather,
    MPIDI_SHM_neighbor_allgatherv,
    MPIDI_SHM_neighbor_alltoall,
    MPIDI_SHM_neighbor_alltoallv,
    MPIDI_SHM_neighbor_alltoallw,
    MPIDI_SHM_ineighbor_allgather,
    MPIDI_SHM_ineighbor_allgatherv,
    MPIDI_SHM_ineighbor_alltoall,
    MPIDI_SHM_ineighbor_alltoallv,
    MPIDI_SHM_ineighbor_alltoallw,
    MPIDI_SHM_ibarrier,
    MPIDI_SHM_ibcast,
    MPIDI_SHM_iallgather,
    MPIDI_SHM_iallgatherv,
    MPIDI_SHM_iallreduce,
    MPIDI_SHM_ialltoall,
    MPIDI_SHM_ialltoallv,
    MPIDI_SHM_ialltoallw,
    MPIDI_SHM_iexscan,
    MPIDI_SHM_igather,
    MPIDI_SHM_igatherv,
    MPIDI_SHM_ireduce_scatter_block,
    MPIDI_SHM_ireduce_scatter,
    MPIDI_SHM_ireduce,
    MPIDI_SHM_iscan,
    MPIDI_SHM_iscatter,
    MPIDI_SHM_iscatterv,
};
#endif
