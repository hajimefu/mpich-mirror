/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#ifndef SHM_DIRECT
#define SHM_DISABLE_INLINES
#include <mpidimpl.h>
#include "ch4_shm_direct.h"
MPIDI_shm_funcs_t MPIDI_shm_stub_funcs = {
    MPIDI_shm_init,
    MPIDI_shm_finalize,
    MPIDI_shm_progress,
    MPIDI_shm_reg_hdr_handler,
    MPIDI_shm_comm_connect,
    MPIDI_shm_comm_disconnect,
    MPIDI_shm_open_port,
    MPIDI_shm_close_port,
    MPIDI_shm_comm_accept,
    MPIDI_shm_send_am_hdr,
    MPIDI_shm_inject_am_hdr,
    MPIDI_shm_send_am,
    MPIDI_shm_inject_am,
    MPIDI_shm_send_amv,
    MPIDI_shm_inject_amv,
    MPIDI_shm_send_am_hdr_reply,
    MPIDI_shm_inject_am_hdr_reply,
    MPIDI_shm_send_am_reply,
    MPIDI_shm_inject_am_reply,
    MPIDI_shm_send_amv_reply,
    MPIDI_shm_inject_amv_reply,
    MPIDI_shm_am_hdr_max_sz,
    MPIDI_shm_am_inject_max_sz,
    MPIDI_shm_comm_get_lpid,
    MPIDI_shm_gpid_get,
    MPIDI_shm_get_node_id,
    MPIDI_shm_get_max_node_id,
    MPIDI_shm_getallincomm,
    MPIDI_shm_gpid_tolpidarray,
    MPIDI_shm_create_intercomm_from_lpids,
    MPIDI_shm_comm_create,
    MPIDI_shm_comm_destroy,
    MPIDI_shm_request_create,
    MPIDI_shm_request_release
};

MPIDI_shm_native_funcs_t MPIDI_shm_native_stub_funcs = {
    MPIDI_shm_send,
    MPIDI_shm_ssend,
    MPIDI_shm_startall,
    MPIDI_shm_send_init,
    MPIDI_shm_ssend_init,
    MPIDI_shm_rsend_init,
    MPIDI_shm_bsend_init,
    MPIDI_shm_isend,
    MPIDI_shm_issend,
    MPIDI_shm_cancel_send,
    MPIDI_shm_recv_init,
    MPIDI_shm_recv,
    MPIDI_shm_irecv,
    MPIDI_shm_imrecv,
    MPIDI_shm_cancel_recv,
    MPIDI_shm_alloc_mem,
    MPIDI_shm_free_mem,
    MPIDI_shm_improbe,
    MPIDI_shm_iprobe,
    MPIDI_shm_win_set_info,
    MPIDI_shm_win_shared_query,
    MPIDI_shm_put,
    MPIDI_shm_win_start,
    MPIDI_shm_win_complete,
    MPIDI_shm_win_post,
    MPIDI_shm_win_wait,
    MPIDI_shm_win_test,
    MPIDI_shm_win_lock,
    MPIDI_shm_win_unlock,
    MPIDI_shm_win_get_info,
    MPIDI_shm_get,
    MPIDI_shm_win_free,
    MPIDI_shm_win_fence,
    MPIDI_shm_win_create,
    MPIDI_shm_accumulate,
    MPIDI_shm_win_attach,
    MPIDI_shm_win_allocate_shared,
    MPIDI_shm_rput,
    MPIDI_shm_win_flush_local,
    MPIDI_shm_win_detach,
    MPIDI_shm_compare_and_swap,
    MPIDI_shm_raccumulate,
    MPIDI_shm_rget_accumulate,
    MPIDI_shm_fetch_and_op,
    MPIDI_shm_win_allocate,
    MPIDI_shm_win_flush,
    MPIDI_shm_win_flush_local_all,
    MPIDI_shm_win_unlock_all,
    MPIDI_shm_win_create_dynamic,
    MPIDI_shm_rget,
    MPIDI_shm_win_sync,
    MPIDI_shm_win_flush_all,
    MPIDI_shm_get_accumulate,
    MPIDI_shm_win_lock_all
};
#endif
