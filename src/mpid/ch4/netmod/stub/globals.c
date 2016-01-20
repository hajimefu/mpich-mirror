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
MPIDI_CH4_NM_funcs_t MPIDI_CH4_NM_stub_funcs = {
    MPIDI_CH4_NM_init,
    MPIDI_CH4_NM_finalize,
    MPIDI_CH4_NM_progress,
    MPIDI_CH4_NM_comm_connect,
    MPIDI_CH4_NM_comm_disconnect,
    MPIDI_CH4_NM_open_port,
    MPIDI_CH4_NM_close_port,
    MPIDI_CH4_NM_comm_accept,
    MPIDI_CH4_NM_comm_get_lpid,
    MPIDI_CH4_NM_gpid_get,
    MPIDI_CH4_NM_get_node_id,
    MPIDI_CH4_NM_get_max_node_id,
    MPIDI_CH4_NM_getallincomm,
    MPIDI_CH4_NM_gpid_tolpidarray,
    MPIDI_CH4_NM_create_intercomm_from_lpids,
    MPIDI_CH4_NM_comm_create,
    MPIDI_CH4_NM_comm_destroy,
    MPIDI_CH4_NM_request_create,
    MPIDI_CH4_NM_request_release,
    MPIDI_CH4_NM_reg_hdr_handler,
    MPIDI_CH4_NM_send_am_hdr,
    MPIDI_CH4_NM_inject_am_hdr,
    MPIDI_CH4_NM_send_am,
    MPIDI_CH4_NM_send_amv,
    MPIDI_CH4_NM_send_amv_hdr,
    MPIDI_CH4_NM_send_am_hdr_reply,
    MPIDI_CH4_NM_inject_am_hdr_reply,
    MPIDI_CH4_NM_send_am_reply,
    MPIDI_CH4_NM_send_amv_reply,
    MPIDI_CH4_NM_am_hdr_max_sz,
    MPIDI_CH4_NM_am_inject_max_sz
};

MPIDI_CH4_NM_native_funcs_t MPIDI_CH4_NM_native_stub_funcs = {
    MPIDI_CH4_NM_send,
    MPIDI_CH4_NM_ssend,
    MPIDI_CH4_NM_startall,
    MPIDI_CH4_NM_send_init,
    MPIDI_CH4_NM_ssend_init,
    MPIDI_CH4_NM_rsend_init,
    MPIDI_CH4_NM_bsend_init,
    MPIDI_CH4_NM_isend,
    MPIDI_CH4_NM_issend,
    MPIDI_CH4_NM_cancel_send,
    MPIDI_CH4_NM_recv_init,
    MPIDI_CH4_NM_recv,
    MPIDI_CH4_NM_irecv,
    MPIDI_CH4_NM_imrecv,
    MPIDI_CH4_NM_cancel_recv,
    MPIDI_CH4_NM_alloc_mem,
    MPIDI_CH4_NM_free_mem,
    MPIDI_CH4_NM_improbe,
    MPIDI_CH4_NM_iprobe,
    MPIDI_CH4_NM_win_set_info,
    MPIDI_CH4_NM_win_shared_query,
    MPIDI_CH4_NM_put,
    MPIDI_CH4_NM_win_start,
    MPIDI_CH4_NM_win_complete,
    MPIDI_CH4_NM_win_post,
    MPIDI_CH4_NM_win_wait,
    MPIDI_CH4_NM_win_test,
    MPIDI_CH4_NM_win_lock,
    MPIDI_CH4_NM_win_unlock,
    MPIDI_CH4_NM_win_get_info,
    MPIDI_CH4_NM_get,
    MPIDI_CH4_NM_win_free,
    MPIDI_CH4_NM_win_fence,
    MPIDI_CH4_NM_win_create,
    MPIDI_CH4_NM_accumulate,
    MPIDI_CH4_NM_win_attach,
    MPIDI_CH4_NM_win_allocate_shared,
    MPIDI_CH4_NM_rput,
    MPIDI_CH4_NM_win_flush_local,
    MPIDI_CH4_NM_win_detach,
    MPIDI_CH4_NM_compare_and_swap,
    MPIDI_CH4_NM_raccumulate,
    MPIDI_CH4_NM_rget_accumulate,
    MPIDI_CH4_NM_fetch_and_op,
    MPIDI_CH4_NM_win_allocate,
    MPIDI_CH4_NM_win_flush,
    MPIDI_CH4_NM_win_flush_local_all,
    MPIDI_CH4_NM_win_unlock_all,
    MPIDI_CH4_NM_win_create_dynamic,
    MPIDI_CH4_NM_rget,
    MPIDI_CH4_NM_win_sync,
    MPIDI_CH4_NM_win_flush_all,
    MPIDI_CH4_NM_get_accumulate,
    MPIDI_CH4_NM_win_lock_all,
    MPIDI_CH4_NM_rank_is_local
};
#endif
