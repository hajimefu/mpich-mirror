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
#ifndef NETMOD_OFI_AM_IMPL_H_INCLUDED
#define NETMOD_OFI_AM_IMPL_H_INCLUDED

#include "impl.h"

#define MPIDI_AM_Win_request_complete(req)                 \
    ({							\
	int count;					\
	MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)	\
		    == MPID_REQUEST);			\
	MPIU_Object_release_ref(req, &count);		\
	MPIU_Assert(count >= 0);			\
	if (count == 0)					\
	{						\
	    MPIU_Free(req->noncontig);			\
	    MPIDI_Win_request_tls_free(req);		\
	}						\
    })

#define MPIDI_AM_Win_request_alloc_and_init(req,count,extra)       \
  ({                                                            \
    MPIDI_Win_request_tls_alloc(req);                           \
    MPIU_Assert(req != NULL);                                   \
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)                \
                == MPID_REQUEST);                               \
    MPIU_Object_set_ref(req, count);                            \
    memset((char*)req+MPIDI_REQUEST_HDR_SIZE, 0,                \
           sizeof(MPIDI_Win_request)-                           \
           MPIDI_REQUEST_HDR_SIZE);                             \
    req->noncontig = (MPIDI_Win_noncontig*)MPIU_Calloc(1,(extra)+sizeof(*(req->noncontig))); \
  })

static inline int MPIDI_netmod_progress_do_queue(void *netmod_context);
#define FI_RC_RETRY_AM(FUNC,STR)					\
	do {								\
		ssize_t _ret;                                           \
		do {							\
			_ret = FUNC;                                    \
			if(likely(_ret==0)) break;			\
			MPIR_ERR_##CHKANDJUMP4(_ret != -FI_EAGAIN,	\
					       mpi_errno,		\
					       MPI_ERR_OTHER,		\
					       "**ofi_"#STR,		\
					       "**ofi_"#STR" %s %d %s %s", \
					       __SHORT_FILE__,		\
					       __LINE__,		\
					       FCNAME,			\
					       fi_strerror(-_ret));	\
				mpi_errno = MPIDI_netmod_progress_do_queue(NULL);\
				if(mpi_errno != MPI_SUCCESS)		\
					MPIR_ERR_POP(mpi_errno);	\
		} while (_ret == -FI_EAGAIN);				\
	} while (0)


static inline MPID_Request *MPIDI_AM_request_alloc_and_init(int count)
{
    MPID_Request *req;
    req = (MPID_Request *) MPIU_Handle_obj_alloc(&MPIDI_Request_mem);
    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPID_cc_set(&req->cc, 1);
    req->cc_ptr = &req->cc;
    MPIU_Object_set_ref(req, count);
    req->greq_fns = NULL;
    MPIR_STATUS_SET_COUNT(req->status, 0);
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);
    req->status.MPI_SOURCE = MPI_UNDEFINED;
    req->status.MPI_TAG = MPI_UNDEFINED;
    req->status.MPI_ERROR = MPI_SUCCESS;
    req->comm = NULL;
    AMREQ_OFI(req, req_hdr) = NULL;
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_am_ofi_clear_req
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_netmod_am_ofi_clear_req(MPID_Request *sreq)
{
    MPIDI_am_ofi_req_hdr_t *req_hdr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_AM_OFI_CLEAR_REQ);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_AM_OFI_CLEAR_REQ);

    req_hdr = AMREQ_OFI(sreq, req_hdr);
    if (!req_hdr)
        return;

    if (req_hdr->am_hdr != &req_hdr->am_hdr_buf[0]) {
        MPIU_Free(req_hdr->am_hdr);
    }
    MPIDI_CH4R_release_buf(req_hdr);
    AMREQ_OFI(sreq, req_hdr) = NULL;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_AM_OFI_CLEAR_REQ);
    return;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_am_ofi_init_req
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_am_ofi_init_req(const void *am_hdr,
                                               size_t am_hdr_sz,
                                               MPID_Request *sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_am_ofi_req_hdr_t *req_hdr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_AM_OFI_INIT_REQ);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_AM_OFI_INIT_REQ);

    if (AMREQ_OFI(sreq, req_hdr) == NULL) {
        req_hdr = (MPIDI_am_ofi_req_hdr_t *)
            MPIDI_CH4R_get_buf(MPIDI_Global.buf_pool);
        MPIU_Assert(req_hdr);
        AMREQ_OFI(sreq, req_hdr) = req_hdr;

        req_hdr->am_hdr = (void *) &req_hdr->am_hdr_buf[0];
        req_hdr->am_hdr_sz = MPIDI_MAX_AM_HDR_SZ;
    } else {
        req_hdr = AMREQ_OFI(sreq, req_hdr);
    }

    if (am_hdr_sz > req_hdr->am_hdr_sz) {
        if (req_hdr->am_hdr != &req_hdr->am_hdr_buf[0])
            MPIU_Free(req_hdr->am_hdr);
        req_hdr->am_hdr = MPIU_Malloc(am_hdr_sz);
        MPIU_Assert(req_hdr->am_hdr);
        req_hdr->am_hdr_sz = am_hdr_sz;
    }

    if (am_hdr) {
        MPIU_Memcpy(req_hdr->am_hdr, am_hdr, am_hdr_sz);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_AM_OFI_INIT_REQ);
    return mpi_errno;
}

static inline int MPIDI_netmod_repost_buffer(void* buf)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_REPOST_BUFFER);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_REPOST_BUFFER);
    FI_RC_RETRY_AM(fi_recvmsg(MPIDI_Global.ep, (struct fi_msg *) buf,
                              FI_MULTI_RECV | FI_COMPLETION), repost);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_REPOST_BUFFER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_progress_do_queue
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_progress_do_queue(void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, ret;
    struct fi_cq_data_entry cq_entry;
    struct fi_cq_err_entry cq_err_entry;
    fi_addr_t source;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_PROGRESS);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_PROGRESS);

    ret = fi_cq_readfrom(MPIDI_Global.p2p_cq, &cq_entry, 1, &source);
    if (ret == -FI_EAGAIN)
        goto fn_exit;

    if (ret < 0) {
        fi_cq_readerr(MPIDI_Global.p2p_cq, &cq_err_entry, 0);
        fprintf(stderr, "fi_cq_read failed with error: %s\n", fi_strerror(cq_err_entry.err));
        goto fn_fail;
    }

    if (((MPIDI_Global.cq_buff_head + 1) %
         MPIDI_NUM_CQ_BUFFERED == MPIDI_Global.cq_buff_tail) ||
        !slist_empty(&MPIDI_Global.cq_buff_list)) {
        struct cq_list *list_entry = (struct cq_list *) MPIU_Malloc(sizeof(struct cq_list));
        MPIU_Assert(list_entry);
        list_entry->cq_entry = cq_entry;
        list_entry->source = source;
        slist_insert_tail(&list_entry->entry, &MPIDI_Global.cq_buff_list);
    } else {
        MPIDI_Global.cq_buffered[MPIDI_Global.cq_buff_head].cq_entry = cq_entry;
        MPIDI_Global.cq_buffered[MPIDI_Global.cq_buff_head].source = source;
        MPIDI_Global.cq_buff_head = (MPIDI_Global.cq_buff_head + 1) % MPIDI_NUM_CQ_BUFFERED;
    }

    if ((cq_entry.flags & FI_RECV) &&
        (cq_entry.flags & FI_MULTI_RECV)) {
        mpi_errno = MPIDI_netmod_repost_buffer(cq_entry.op_context);
        if (mpi_errno) MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_PROGRESS);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_ofi_do_send_am_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_ofi_do_send_am_hdr(int64_t rank,
                                                  int     handler_id,
                                                  const void *am_hdr,
                                                  size_t am_hdr_sz, MPID_Request * sreq)
{
    struct iovec iov[2];
    MPIDI_AM_OFI_hdr_t *msg_hdr;
    int mpi_errno = MPI_SUCCESS, c;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_SEND_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_SEND_AM_HDR);

    mpi_errno = MPIDI_netmod_am_ofi_init_req(am_hdr, am_hdr_sz, sreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    msg_hdr = &AMREQ_OFI_HDR(sreq, msg_hdr);
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz = am_hdr_sz;
    msg_hdr->data_sz = 0;
    msg_hdr->am_type = MPIDI_AMTYPE_SHORT_HDR;

    AMREQ_OFI_HDR(sreq, pack_buffer) = NULL;
    MPID_cc_incr(sreq->cc_ptr, &c);

    iov[0].iov_base = msg_hdr;
    iov[0].iov_len = sizeof(*msg_hdr);

    MPIU_Assert((sizeof(*msg_hdr) + am_hdr_sz) <= MPIDI_MAX_SHORT_SEND_SZ);
    iov[1].iov_base = AMREQ_OFI_HDR(sreq, am_hdr);
    iov[1].iov_len = am_hdr_sz;
    AMREQ_OFI(sreq, event_id) = MPIDI_EVENT_AM_SEND;
    FI_RC_RETRY_AM(fi_sendv(MPIDI_Global.ep, iov, NULL, 2, rank, &AMREQ_OFI(sreq, context)), sendv);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DO_SEND_AM_HDR);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_ofi_send_am_long
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_ofi_send_am_long(int64_t rank, int handler_id,
                                                const void *am_hdr, size_t am_hdr_sz,
                                                const void *data, size_t data_sz,
                                                MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_AM_OFI_hdr_t *msg_hdr;
    MPIDI_OFI_lmt_msg_pyld_t *lmt_info;
    struct iovec iov[3];
    uint64_t index;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AM_LONG);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AM_LONG);

    msg_hdr = &AMREQ_OFI_HDR(sreq, msg_hdr);
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz = am_hdr_sz;
    msg_hdr->data_sz = data_sz;
    msg_hdr->am_type = MPIDI_AMTYPE_LMT_REQ;

    lmt_info = &AMREQ_OFI_HDR(sreq, lmt_info);
    lmt_info->src_offset = (uint64_t) 0; /* TODO: Set to data if MR_BASIC */
    lmt_info->sreq_ptr = (uint64_t) sreq;
    /* Always allocates RMA ID from COMM_WORLD as the actual associated communicator
       is not available here */
    index = MPIDI_OFI_Index_allocator_alloc(COMM_OFI(MPIR_Process.comm_world).rma_id_allocator);
    MPIU_Assert(index < MPIDI_Global.max_huge_rmas);
    lmt_info->rma_key = index << MPIDI_Global.huge_rma_shift;

    MPID_cc_incr(sreq->cc_ptr, &c); /* send completion */
    MPID_cc_incr(sreq->cc_ptr, &c); /* lmt ack handler */
    MPIU_Assert((sizeof(*msg_hdr) + sizeof(*lmt_info) + am_hdr_sz) <= MPIDI_MAX_SHORT_SEND_SZ);

    FI_RC(fi_mr_reg(MPIDI_Global.domain,
                    data,
                    data_sz,
                    FI_REMOTE_READ,
                    0ULL,
                    lmt_info->rma_key,
                    0ULL,
                    &AMREQ_OFI_HDR(sreq, lmt_mr),
                    NULL), mr_reg);

    iov[0].iov_base = msg_hdr;
    iov[0].iov_len = sizeof(*msg_hdr);

    iov[1].iov_base = AMREQ_OFI_HDR(sreq, am_hdr);
    iov[1].iov_len = am_hdr_sz;

    iov[2].iov_base = lmt_info;
    iov[2].iov_len = sizeof(*lmt_info);
    AMREQ_OFI(sreq, event_id) = MPIDI_EVENT_AM_SEND;
    FI_RC_RETRY_AM(fi_sendv(MPIDI_Global.ep, iov, NULL, 3, rank, &AMREQ_OFI(sreq, context)), sendv);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AM_LONG);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_ofi_send_am_short
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_ofi_send_am_short(int64_t rank, int handler_id,
                                                 const void *am_hdr, size_t am_hdr_sz,
                                                 const void *data, MPI_Count count,
                                                 MPID_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_AM_OFI_hdr_t *msg_hdr;
    struct iovec iov[3];

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AM_SHORT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AM_SHORT);

    msg_hdr = &AMREQ_OFI_HDR(sreq, msg_hdr);
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz = am_hdr_sz;
    msg_hdr->data_sz = count;
    msg_hdr->am_type = MPIDI_AMTYPE_SHORT;

    iov[0].iov_base = msg_hdr;
    iov[0].iov_len = sizeof(*msg_hdr);

    iov[1].iov_base = AMREQ_OFI_HDR(sreq, am_hdr);
    iov[1].iov_len = am_hdr_sz;

    iov[2].iov_base = (void *) data;
    iov[2].iov_len = count;

    MPID_cc_incr(sreq->cc_ptr, &c);
    AMREQ_OFI(sreq, event_id) = MPIDI_EVENT_AM_SEND;
    FI_RC_RETRY_AM(fi_sendv(MPIDI_Global.ep, iov, NULL, 3, rank, &AMREQ_OFI(sreq, context)), sendv);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AM_SHORT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_ofi_do_send_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_ofi_do_send_am(int64_t rank, int handler_id,
                                              const void *am_hdr, size_t am_hdr_sz,
                                              const void *buf, size_t count,
                                              MPI_Datatype datatype, MPID_Request * sreq)
{
    char *send_buf;
    int dt_contig, mpi_errno = MPI_SUCCESS;
    MPIDI_msg_sz_t data_sz;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;
    MPI_Aint last;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DO_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DO_SEND_AM);

    mpi_errno = MPIDI_netmod_am_ofi_init_req(am_hdr, am_hdr_sz, sreq);
    if (mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    send_buf = (char *) buf + dt_true_lb;

    if (!dt_contig) {
        MPIDI_msg_sz_t segment_first;
        struct MPID_Segment *segment_ptr;
        segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Send MPID_Segment_alloc");
        MPID_Segment_init(buf, count, datatype, segment_ptr, 0);
        segment_first = 0;
        last = data_sz;
        AMREQ_OFI_HDR(sreq, pack_buffer) = (char *) MPIU_Malloc(data_sz);
        MPIR_ERR_CHKANDJUMP1(AMREQ_OFI_HDR(sreq, pack_buffer) == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Send Pack buffer alloc");
        MPID_Segment_pack(segment_ptr, segment_first, &last, AMREQ_OFI_HDR(sreq, pack_buffer));
        MPID_Segment_free(segment_ptr);
        send_buf = (char *) AMREQ_OFI_HDR(sreq, pack_buffer);
    }
    else {
        AMREQ_OFI_HDR(sreq, pack_buffer) = NULL;
    }

    mpi_errno = ((am_hdr_sz + data_sz + sizeof(MPIDI_AM_OFI_hdr_t)) < MPIDI_MAX_SHORT_SEND_SZ) ?
        MPIDI_netmod_ofi_send_am_short(rank, handler_id, AMREQ_OFI_HDR(sreq, am_hdr),
                                       am_hdr_sz, send_buf, data_sz, sreq) :
        MPIDI_netmod_ofi_send_am_long(rank, handler_id, AMREQ_OFI_HDR(sreq, am_hdr),
                                      am_hdr_sz, send_buf, data_sz, sreq);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_DO_SEND_AM);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_netmod_do_inject(int64_t     rank,
                                         int         handler_id,
                                         const void *am_hdr,
                                         size_t      am_hdr_sz,
                                         void       *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_AM_OFI_hdr_t msg_hdr;
    struct fi_msg msg;
    struct iovec msg_iov[2];

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_INJECT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_INJECT);

    MPIU_Assert(am_hdr_sz + sizeof(msg_hdr) < MPIDI_Global.max_buffered_send);

    msg_hdr.handler_id  = handler_id;
    msg_hdr.am_hdr_sz   = am_hdr_sz;
    msg_hdr.data_sz     = 0;
    msg_hdr.am_type     = MPIDI_AMTYPE_SHORT;

    msg_iov[0].iov_base = (void *) &msg_hdr;
    msg_iov[0].iov_len  = sizeof(msg_hdr);

    msg_iov[1].iov_base = (void *) am_hdr;
    msg_iov[1].iov_len  = am_hdr_sz;

    msg.msg_iov   = &msg_iov[0];
    msg.desc      = NULL;
    msg.iov_count = 2;
    msg.context   = NULL;
    msg.addr      = (fi_addr_t)rank;
    FI_RC_RETRY_AM(fi_sendmsg(MPIDI_Global.ep, &msg, FI_INJECT), send);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DO_INJECT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline void MPIDI_AM_netmod_request_complete(MPID_Request *req)
{
    int count;
    MPID_cc_decr(req->cc_ptr, &count);
    MPIU_Assert(count >= 0);
    if (count == 0)
        MPIDI_Request_release(req);
}

static inline MPID_Request *MPIDI_AM_netmod_request_create(void)
{
    return MPIDI_AM_request_alloc_and_init(1);
}

#endif /*NETMOD_OFI_AM_IMPL_H_INCLUDED */
