/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
#include "netmod_macros.h"

/* discover network resources, creates num_contexts (default 1), optionally PMI_Put business card */
MPIDI_NETMOD_API(int,init,
                 (int)        rank,
                 (int)        size,
                 (int)        appnum,
                 (int*)       tag_ub,
                 (MPID_Comm*) comm_world,
                 (MPID_Comm*) comm_self,
                 (int)        spawned,
                 (int)        num_contexts,
                 (void**)     netmod_contexts); /* multiple communication contexts enable fine-grained progress */

/* teardown all network contexts, shutdown network resources */
MPIDI_NETMOD_API(int,finalize,
                 (void));

/* make progress on an individual netmod context
 * note: revisit decision to do single v-> array of contexts in progress
 *         or have multiple progress functions
 */
MPIDI_NETMOD_API(int,progress,
                 (void *)netmod_context,
                 (int) blocking);

/* called by CH4 and netmods to register header types */
MPIDI_NETMOD_API(int,reg_hdr_handler,
		 (int) handler_id,
		 (MPIDI_netmod_am_origin_handler_fn) origin_handler_fn,
		 (MPIDI_netmod_am_target_handler_fn) target_handler_fn);

/* for dynamic processes */
MPIDI_NETMOD_API(int,comm_connect,
                 (const char *) port_name,
                 (MPID_Info *)  info,
                 (int)          root,
                 (MPID_Comm *)  comm,
                 (MPID_Comm **) newcomm_ptr);

MPIDI_NETMOD_API(int,comm_disconnect,
                 (MPID_Comm *) comm_ptr);

MPIDI_NETMOD_API(int,open_port,
                 (MPID_Info *) info_ptr,
                 (char *)      port_name);

MPIDI_NETMOD_API(int,close_port,
                 (const char *) port_name);

MPIDI_NETMOD_API(int,comm_accept,
                 (const char *) port_name,
                 (MPID_Info *)  info,
                 (int)          root,
                 (MPID_Comm *)  comm,
                 (MPID_Comm **) newcomm_ptr);

/* active message send header */
MPIDI_NETMOD_API(int,send_am_hdr,
                 (int)            rank,
                 (MPID_Comm *)    comm,
                 (int)            handler_id,
                 (const void*)    am_hdr,
                 (size_t)         am_hdr_sz,
                 (MPID_Request *) sreq,
                 (void *)         netmod_context);

/* active message inject header */
MPIDI_NETMOD_API(int,inject_am_hdr,
                 (int)            rank,
                 (MPID_Comm *)    comm,
                 (int)            handler_id,
                 (const void*)    am_hdr,
                 (size_t)         am_hdr_sz,
                 (void *)         netmod_context);


/* active message send */
MPIDI_NETMOD_API(int,send_am,
                 (int)            rank,
                 (MPID_Comm *)    comm,
                 (int)            handler_id,
                 (const void *)   am_hdr,
                 (size_t)         am_hdr_sz,
                 (const void *)   data,
                 (MPI_Count)      count,
                 (MPI_Datatype)   datatype,
                 (MPID_Request *) sreq,           /* sreq needs netmod area, AM send completion is triggered by calling sreq->am_sent */
                 (void *)         netmod_context);/* CH4 selects netmod context via some policy */


/* vector version of active message send */
MPIDI_NETMOD_API(int,send_amv,
                 (int)            rank,
                 (MPID_Comm *)    comm,
                 (int)            handler_id,
                 (struct iovec *) am_hdrs,
                 (size_t)         iov_len,
                 (const void *)   data,
                 (MPI_Count)      count,
                 (MPI_Datatype)   datatype,
                 (MPID_Request *) sreq,
                 (void *)         netmod_context);

/* vector version of active message send hdr */
MPIDI_NETMOD_API(int,send_amv_hdr,
                 (int)            rank,
                 (MPID_Comm *)    comm,
                 (int)            handler_id,
                 (struct iovec *) am_hdrs,
                 (size_t)         iov_len,
                 (MPID_Request *) sreq,
                 (void *)         netmod_context);

/* active message send am hdr reply */
MPIDI_NETMOD_API(int,send_am_hdr_reply,
                 (void *)         reply_token,     /* local pointer to netmod specific information */
                 (int)            handler_id,
                 (const void *)   am_hdr,
                 (size_t)         am_hdr_sz,
                 (MPID_Request *) sreq);

/* active message inject hdr reply */
MPIDI_NETMOD_API(int,inject_am_hdr_reply,
                 (void *)         reply_token,     /* local pointer to netmod specific information */
                 (int)            handler_id,
                 (const void *)   am_hdr,
                 (size_t)         am_hdr_sz);

/* active message reply */
MPIDI_NETMOD_API(int,send_am_reply,
                 (void *)         reply_token, /* local pointer to netmod specific information */
                 (int)            handler_id,
                 (const void *)   am_hdr,
                 (size_t)         am_hdr_sz,
                 (const void *)   data,
                 (MPI_Count)      count,
                 (MPI_Datatype)   datatype,
                 (MPID_Request *) sreq);

/* vector version of reply */
MPIDI_NETMOD_API(int,send_amv_reply,
                 (void *)         reply_token,        /* local pointer to netmod specific information */
                 (int)            handler_id,
                 (struct iovec *) am_hdr,
                 (size_t)         iov_len,
                 (const void *)   data,
                 (MPI_Count)      count,
                 (MPI_Datatype)   datatype,
                 (MPID_Request *) sreq);

MPIDI_NETMOD_API(size_t, am_hdr_max_sz,
                 (void));
MPIDI_NETMOD_API(size_t, am_inject_max_sz,
                 (void));

MPIDI_NETMOD_API(int,comm_get_lpid,
                 (MPID_Comm *) comm_ptr,
                 (int)         idx,
                 (int *)       lpid_ptr,
                 (MPIU_BOOL)   is_remote);

MPIDI_NETMOD_API(int,gpid_get,
                 (MPID_Comm *) comm_ptr,
                 (int)         rank,
                 (MPID_Gpid *) gpid);

MPIDI_NETMOD_API(int,get_node_id,
                 (MPID_Comm *)      comm,
                 (int)              rank,
                 (MPID_Node_id_t *) id_p);

MPIDI_NETMOD_API(int,get_max_node_id,
                 (MPID_Comm *)      comm,
                 (MPID_Node_id_t *) max_id_p);

MPIDI_NETMOD_API(int,getallincomm,
                 (MPID_Comm *)       comm_ptr,
                 (int)               local_size,
                 (MPID_Gpid_array_t) local_gpid,
                 (int *)             singlePG);

MPIDI_NETMOD_API(int,gpid_tolpidarray,
                 (int)               size,
                 (MPID_Gpid_array_t) gpid,
                 (intarray_t)        lpid);

MPIDI_NETMOD_API(int,create_intercomm_from_lpids,
                 (MPID_Comm *)      newcomm_ptr,
                 (int)              size,
                 (const intarray_t) lpids);

MPIDI_NETMOD_API(int,comm_create,
                 (MPID_Comm *) comm);

MPIDI_NETMOD_API(int,comm_destroy,
                 (MPID_Comm *) comm);

MPIDI_NETMOD_API(MPID_Request *,request_create,
                 (void));

MPIDI_NETMOD_API(void,request_release,
                 (MPID_Request *) req);

MPIDI_NETMOD_API_NATIVE(int,send,
                        (const void *)    buf,
                        (int)             count,
                        (MPI_Datatype)    datatype,
                        (int)             rank,
                        (int)             tag,
                        (MPID_Comm *)     comm,
                        (int)             context_offset,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,ssend,
                        (const void *)    buf,
                        (int)             count,
                        (MPI_Datatype)    datatype,
                        (int)             rank,
                        (int)             tag,
                        (MPID_Comm *)     comm,
                        (int)             context_offset,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,startall,
                        (int)                    count,
                        (MPID_Request_array_t)   requests);

MPIDI_NETMOD_API_NATIVE(int,send_init,
                        (const void *)    buf,
                        (int)             count,
                        (MPI_Datatype)    datatype,
                        (int)             rank,
                        (int)             tag,
                        (MPID_Comm *)     comm,
                        (int)             context_offset,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,ssend_init,
                        (const void *)    buf,
                        (int)             count,
                        (MPI_Datatype)    datatype,
                        (int)             rank,
                        (int)             tag,
                        (MPID_Comm *)     comm,
                        (int)             context_offset,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,rsend_init,
                        (const void *)    buf,
                        (int)             count,
                        (MPI_Datatype)    datatype,
                        (int)             rank,
                        (int)             tag,
                        (MPID_Comm *)     comm,
                        (int)             context_offset,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,bsend_init,
                        (const void *)    buf,
                        (int)             count,
                        (MPI_Datatype)    datatype,
                        (int)             rank,
                        (int)             tag,
                        (MPID_Comm *)     comm,
                        (int)             context_offset,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,isend,
                        (const void *)    buf,
                        (int)             count,
                        (MPI_Datatype)    datatype,
                        (int)             rank,
                        (int)             tag,
                        (MPID_Comm *)     comm,
                        (int)             context_offset,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,issend,
                        (const void *)    buf,
                        (int)             count,
                        (MPI_Datatype)    datatype,
                        (int)             rank,
                        (int)             tag,
                        (MPID_Comm *)     comm,
                        (int)             context_offset,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,cancel_send,
                        (MPID_Request *) sreq);

MPIDI_NETMOD_API_NATIVE(int,recv_init,
                        (void *)          buf,
                        (int)             count,
                        (MPI_Datatype)    datatype,
                        (int)             rank,
                        (int)             tag,
                        (MPID_Comm *)     comm,
                        (int)             context_offset,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,recv,
                        (void *)          buf,
                        (int)             count,
                        (MPI_Datatype)    datatype,
                        (int)             rank,
                        (int)             tag,
                        (MPID_Comm *)     comm,
                        (int)             context_offset,
                        (MPI_Status *)    status,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,irecv,
                        (void *)          buf,
                        (int)             count,
                        (MPI_Datatype)    datatype,
                        (int)             rank,
                        (int)             tag,
                        (MPID_Comm *)     comm,
                        (int)             context_offset,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,imrecv,
                        (void *)          buf,
                        (int)             count,
                        (MPI_Datatype)    datatype,
                        (MPID_Request *)  message,
                        (MPID_Request **) rreqp);

MPIDI_NETMOD_API_NATIVE(int,cancel_recv,
                        (MPID_Request *) rreq);

MPIDI_NETMOD_API_NATIVE(void *,alloc_mem,
                        (size_t) size,
                        (MPID_Info *) info_ptr);

MPIDI_NETMOD_API_NATIVE(int,free_mem,
                        (void *)ptr);

MPIDI_NETMOD_API_NATIVE(int,improbe,
                        (int) source,
                        (int) tag,
                        (MPID_Comm *) comm,
                        (int) context_offset,
                        (int *)flag,
                        (MPID_Request **) message,
                        (MPI_Status *) status);

MPIDI_NETMOD_API_NATIVE(int,iprobe,
                        (int)          source,
                        (int)          tag,
                        (MPID_Comm *)  comm,
                        (int)          context_offset,
                        (int *)        flag,
                        (MPI_Status *) status);

MPIDI_NETMOD_API_NATIVE(int,win_set_info,
                        (MPID_Win *) win,
                        (MPID_Info *) info);

MPIDI_NETMOD_API_NATIVE(int,win_shared_query,
                        (MPID_Win *) win,
                        (int)        rank,
                        (MPI_Aint *) size,
                        (int *)      disp_unit,
                        (void *)     baseptr);

MPIDI_NETMOD_API_NATIVE(int,put,
                        (const void *) origin_addr,
                        (int)          origin_count,
                        (MPI_Datatype) origin_datatype,
                        (int)          target_rank,
                        (MPI_Aint)     target_disp,
                        (int)          target_count,
                        (MPI_Datatype) target_datatype,
                        (MPID_Win *)   win);

MPIDI_NETMOD_API_NATIVE(int,win_start,
                        (MPID_Group *) group,
                        (int)          assert,
                        (MPID_Win *)   win);

MPIDI_NETMOD_API_NATIVE(int,win_complete,
                        (MPID_Win *) win);

MPIDI_NETMOD_API_NATIVE(int,win_post,
                        (MPID_Group *) group,
                        (int)          assert,
                        (MPID_Win *)   win);

MPIDI_NETMOD_API_NATIVE(int,win_wait,
                        (MPID_Win *) win);

MPIDI_NETMOD_API_NATIVE(int,win_test,
                        (MPID_Win *) win,
                        (int *)      flag);

MPIDI_NETMOD_API_NATIVE(int,win_lock,
                        (int)        lock_type,
                        (int)        rank,
                        (int)        assert,
                        (MPID_Win *) win);

MPIDI_NETMOD_API_NATIVE(int,win_unlock,
                        (int)        rank,
                        (MPID_Win *) win);

MPIDI_NETMOD_API_NATIVE(int,win_get_info,
                        (MPID_Win *) win,
                        (MPID_Info **) info_p_p);

MPIDI_NETMOD_API_NATIVE(int,get,
                        (void *)       origin_addr,
                        (int)          origin_count,
                        (MPI_Datatype) origin_datatype,
                        (int)          target_rank,
                        (MPI_Aint)     target_disp,
                        (int)          target_count,
                        (MPI_Datatype) target_datatype,
                        (MPID_Win *)   win);

MPIDI_NETMOD_API_NATIVE(int,win_free,
                        (MPID_Win **) win_ptr);

MPIDI_NETMOD_API_NATIVE(int,win_fence,
                        (int) assert,
                        (MPID_Win *) win);

MPIDI_NETMOD_API_NATIVE(int,win_create,
                        (void *)      base,
                        (MPI_Aint)    length,
                        (int)         disp_unit,
                        (MPID_Info *) info,
                        (MPID_Comm *) comm_ptr,
                        (MPID_Win **) win_ptr);

MPIDI_NETMOD_API_NATIVE(int,accumulate,
                        (const void *) origin_addr,
                        (int)          origin_count,
                        (MPI_Datatype) origin_datatype,
                        (int)          target_rank,
                        (MPI_Aint)     target_disp,
                        (int)          target_count,
                        (MPI_Datatype) target_datatype,
                        (MPI_Op)       op,
                        (MPID_Win *)   win);

MPIDI_NETMOD_API_NATIVE(int,win_attach,
                        (MPID_Win *) win,
                        (void *)     base,
                        (MPI_Aint)   size);

MPIDI_NETMOD_API_NATIVE(int,win_allocate_shared,
                        (MPI_Aint)    size,
                        (int)         disp_unit,
                        (MPID_Info *) info_ptr,
                        (MPID_Comm *) comm_ptr,
                        (void **)     base_ptr,
                        (MPID_Win **) win_ptr);

MPIDI_NETMOD_API_NATIVE(int,rput,
                        (const void *)    origin_addr,
                        (int)             origin_count,
                        (MPI_Datatype)    origin_datatype,
                        (int)             target_rank,
                        (MPI_Aint)        target_disp,
                        (int)             target_count,
                        (MPI_Datatype)    target_datatype,
                        (MPID_Win *)      win,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,win_flush_local,
                        (int) rank,
                        (MPID_Win *) win);

MPIDI_NETMOD_API_NATIVE(int,win_detach,
                        (MPID_Win *)   win,
                        (const void *) base);

MPIDI_NETMOD_API_NATIVE(int,compare_and_swap,
                        (const void *) origin_addr,
                        (const void *) compare_addr,
                        (void *)       result_addr,
                        (MPI_Datatype) datatype,
                        (int)          target_rank,
                        (MPI_Aint)     target_disp,
                        (MPID_Win *)   win);

MPIDI_NETMOD_API_NATIVE(int,raccumulate,
                        (const void *)    origin_addr,
                        (int)             origin_count,
                        (MPI_Datatype)    origin_datatype,
                        (int)             target_rank,
                        (MPI_Aint)        target_disp,
                        (int)             target_count,
                        (MPI_Datatype)    target_datatype,
                        (MPI_Op)          op,
                        (MPID_Win *)      win,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,rget_accumulate,
                        (const void *)    origin_addr,
                        (int)             origin_count,
                        (MPI_Datatype)    origin_datatype,
                        (void *)          result_addr,
                        (int)             result_count,
                        (MPI_Datatype)    result_datatype,
                        (int)             target_rank,
                        (MPI_Aint)        target_disp,
                        (int)             target_count,
                        (MPI_Datatype)    target_datatype,
                        (MPI_Op)          op,
                        (MPID_Win *)      win,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,fetch_and_op,
                        (const void *) origin_addr,
                        (void *)       result_addr,
                        (MPI_Datatype) datatype,
                        (int)          target_rank,
                        (MPI_Aint)     target_disp,
                        (MPI_Op)       op,
                        (MPID_Win *)   win);

MPIDI_NETMOD_API_NATIVE(int,win_allocate,
                        (MPI_Aint)    size,
                        (int)         disp_unit,
                        (MPID_Info *) info,
                        (MPID_Comm *) comm,
                        (void *)      baseptr,
                        (MPID_Win **) win);

MPIDI_NETMOD_API_NATIVE(int,win_flush,
                        (int)        rank,
                        (MPID_Win *) win);

MPIDI_NETMOD_API_NATIVE(int,win_flush_local_all,
                        (MPID_Win *) win);

MPIDI_NETMOD_API_NATIVE(int,win_unlock_all,
                        (MPID_Win *) win);

MPIDI_NETMOD_API_NATIVE(int,win_create_dynamic,
                        (MPID_Info *) info,
                        (MPID_Comm *) comm,
                        (MPID_Win **) win);

MPIDI_NETMOD_API_NATIVE(int,rget,
                        (void *)          origin_addr,
                        (int)             origin_count,
                        (MPI_Datatype)    origin_datatype,
                        (int)             target_rank,
                        (MPI_Aint)        target_disp,
                        (int)             target_count,
                        (MPI_Datatype)    target_datatype,
                        (MPID_Win *)      win,
                        (MPID_Request **) request);

MPIDI_NETMOD_API_NATIVE(int,win_sync,
                        (MPID_Win *) win);

MPIDI_NETMOD_API_NATIVE(int,win_flush_all,
                        (MPID_Win *) win);

MPIDI_NETMOD_API_NATIVE(int,get_accumulate,
                        (const void *) origin_addr,
                        (int)          origin_count,
                        (MPI_Datatype) origin_datatype,
                        (void *)       result_addr,
                        (int)          result_count,
                        (MPI_Datatype) result_datatype,
                        (int)          target_rank,
                        (MPI_Aint)     target_disp,
                        (int)          target_count,
                        (MPI_Datatype) target_datatype,
                        (MPI_Op)       op,
                        (MPID_Win *)   win);

MPIDI_NETMOD_API_NATIVE(int,win_lock_all,
                        (int) assert,
                        (MPID_Win *) win);

MPIDI_NETMOD_API_NATIVE(int, rank_is_local,
                        (int) target,
                        (MPID_Comm *) comm);

#undef MPIDI_NETMOD_API
#undef MPIDI_NETMOD_API_NATIVE
