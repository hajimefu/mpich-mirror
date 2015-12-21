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

#ifndef NETMOD_DIRECT
#define NETMOD_DISABLE_INLINES
#include <mpidimpl.h>
#include "netmod_direct.h"
MPIDI_netmod_funcs_t MPIDI_netmod_ofi_funcs = {
    MPIDI_netmod_init,
    MPIDI_netmod_finalize,
    MPIDI_netmod_progress,
    MPIDI_netmod_reg_hdr_handler,
    MPIDI_netmod_comm_connect,
    MPIDI_netmod_comm_disconnect,
    MPIDI_netmod_open_port,
    MPIDI_netmod_close_port,
    MPIDI_netmod_comm_accept,
    MPIDI_netmod_send_am_hdr,
    MPIDI_netmod_inject_am_hdr,
    MPIDI_netmod_send_am,
    MPIDI_netmod_send_amv,
    MPIDI_netmod_send_amv_hdr,
    MPIDI_netmod_send_am_hdr_reply,
    MPIDI_netmod_inject_am_hdr_reply,
    MPIDI_netmod_send_am_reply,
    MPIDI_netmod_send_amv_reply,
    MPIDI_netmod_am_hdr_max_sz,
    MPIDI_netmod_am_inject_max_sz,
    MPIDI_netmod_comm_get_lpid,
    MPIDI_netmod_gpid_get,
    MPIDI_netmod_get_node_id,
    MPIDI_netmod_get_max_node_id,
    MPIDI_netmod_getallincomm,
    MPIDI_netmod_gpid_tolpidarray,
    MPIDI_netmod_create_intercomm_from_lpids,
    MPIDI_netmod_comm_create,
    MPIDI_netmod_comm_destroy,
    MPIDI_netmod_request_create,
    MPIDI_netmod_request_release
};

MPIDI_netmod_native_funcs_t MPIDI_netmod_native_ofi_funcs = {
    MPIDI_netmod_send,
    MPIDI_netmod_ssend,
    MPIDI_netmod_startall,
    MPIDI_netmod_send_init,
    MPIDI_netmod_ssend_init,
    MPIDI_netmod_rsend_init,
    MPIDI_netmod_bsend_init,
    MPIDI_netmod_isend,
    MPIDI_netmod_issend,
    MPIDI_netmod_cancel_send,
    MPIDI_netmod_recv_init,
    MPIDI_netmod_recv,
    MPIDI_netmod_irecv,
    MPIDI_netmod_imrecv,
    MPIDI_netmod_cancel_recv,
    MPIDI_netmod_alloc_mem,
    MPIDI_netmod_free_mem,
    MPIDI_netmod_improbe,
    MPIDI_netmod_iprobe,
    MPIDI_netmod_win_set_info,
    MPIDI_netmod_win_shared_query,
    MPIDI_netmod_put,
    MPIDI_netmod_win_start,
    MPIDI_netmod_win_complete,
    MPIDI_netmod_win_post,
    MPIDI_netmod_win_wait,
    MPIDI_netmod_win_test,
    MPIDI_netmod_win_lock,
    MPIDI_netmod_win_unlock,
    MPIDI_netmod_win_get_info,
    MPIDI_netmod_get,
    MPIDI_netmod_win_free,
    MPIDI_netmod_win_fence,
    MPIDI_netmod_win_create,
    MPIDI_netmod_accumulate,
    MPIDI_netmod_win_attach,
    MPIDI_netmod_win_allocate_shared,
    MPIDI_netmod_rput,
    MPIDI_netmod_win_flush_local,
    MPIDI_netmod_win_detach,
    MPIDI_netmod_compare_and_swap,
    MPIDI_netmod_raccumulate,
    MPIDI_netmod_rget_accumulate,
    MPIDI_netmod_fetch_and_op,
    MPIDI_netmod_win_allocate,
    MPIDI_netmod_win_flush,
    MPIDI_netmod_win_flush_local_all,
    MPIDI_netmod_win_unlock_all,
    MPIDI_netmod_win_create_dynamic,
    MPIDI_netmod_rget,
    MPIDI_netmod_win_sync,
    MPIDI_netmod_win_flush_all,
    MPIDI_netmod_get_accumulate,
    MPIDI_netmod_win_lock_all,
    MPIDI_netmod_rank_is_local
};
#endif
