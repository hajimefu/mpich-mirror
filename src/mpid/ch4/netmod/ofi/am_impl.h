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

static inline int MPIDI_CH4_NMI_OFI_Progress_do_queue(void *netmod_context);

#define MPIDI_CH4_NMI_OFI_Am_win_request_complete(req)                 \
    ({							\
	int count;					\
	MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)	\
		    == MPID_REQUEST);			\
	MPIU_Object_release_ref(req, &count);		\
	MPIU_Assert(count >= 0);			\
	if (count == 0)					\
	{						\
	    MPL_free(req->noncontig);			\
	    MPIDI_CH4_NMI_OFI_Win_request_t_tls_free(req);		\
	}						\
    })

#define MPIDI_CH4_NMI_OFI_Am_win_request_alloc_and_init(req,count,extra)       \
  ({                                                            \
    MPIDI_CH4_NMI_OFI_Win_request_t_tls_alloc(req);                           \
    MPIU_Assert(req != NULL);                                   \
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)                \
                == MPID_REQUEST);                               \
    MPIU_Object_set_ref(req, count);                            \
    memset((char*)req+MPIDI_REQUEST_HDR_SIZE, 0,                \
           sizeof(MPIDI_CH4_NMI_OFI_Win_request_t)-                           \
           MPIDI_REQUEST_HDR_SIZE);                             \
    req->noncontig = (MPIDI_CH4_NMI_OFI_Win_noncontig_t*)MPL_calloc(1,(extra)+sizeof(*(req->noncontig))); \
  })

#define MPIDI_CH4_NMI_OFI_CALL_RETRY_AM(FUNC,STR)					\
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
				mpi_errno = MPIDI_CH4_NMI_OFI_Progress_do_queue(NULL);\
				if(mpi_errno != MPI_SUCCESS)		\
					MPIR_ERR_POP(mpi_errno);	\
		} while (_ret == -FI_EAGAIN);				\
	} while (0)


static inline MPID_Request *MPIDI_CH4_NMI_OFI_Am_request_alloc_and_init(int count)
{
    MPID_Request *req;
    req = (MPID_Request *) MPIU_Handle_obj_alloc(&MPIDI_Request_mem);
    MPIU_Assert(req != NULL);
    MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle) == MPID_REQUEST);
    MPIR_cc_set(&req->cc, 1);
    req->cc_ptr = &req->cc;
    MPIU_Object_set_ref(req, count);
    req->greq_fns = NULL;
    MPIR_STATUS_SET_COUNT(req->status, 0);
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);
    req->status.MPI_SOURCE = MPI_UNDEFINED;
    req->status.MPI_TAG = MPI_UNDEFINED;
    req->status.MPI_ERROR = MPI_SUCCESS;
    req->comm = NULL;
    MPIDI_CH4_NMI_OFI_AMREQUEST(req, req_hdr) = NULL;
    MPIR_REQUEST_CLEAR_DBG(req);
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Am_clear_request
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void MPIDI_CH4_NMI_OFI_Am_clear_request(MPID_Request *sreq)
{
    MPIDI_CH4_NMI_OFI_Am_request_header_t *req_hdr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_AM_OFI_CLEAR_REQ);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_AM_OFI_CLEAR_REQ);

    req_hdr = MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, req_hdr);

    if(!req_hdr)
        return;

    if(req_hdr->am_hdr != &req_hdr->am_hdr_buf[0]) {
        MPL_free(req_hdr->am_hdr);
    }

    MPIDI_CH4R_release_buf(req_hdr);
    MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, req_hdr) = NULL;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_AM_OFI_CLEAR_REQ);
    return;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Am_init_request
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Am_init_request(const void *am_hdr,
                                                    size_t am_hdr_sz,
                                                    MPID_Request *sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_CH4_NMI_OFI_Am_request_header_t *req_hdr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_AM_OFI_INIT_REQ);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_AM_OFI_INIT_REQ);

    if(MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, req_hdr) == NULL) {
        req_hdr = (MPIDI_CH4_NMI_OFI_Am_request_header_t *)
                  MPIDI_CH4R_get_buf(MPIDI_Global.am_buf_pool);
        MPIU_Assert(req_hdr);
        MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, req_hdr) = req_hdr;

        req_hdr->am_hdr = (void *) &req_hdr->am_hdr_buf[0];
        req_hdr->am_hdr_sz = MPIDI_CH4_NMI_OFI_MAX_AM_HDR_SIZE;
    } else {
        req_hdr = MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, req_hdr);
    }

    if(am_hdr_sz > req_hdr->am_hdr_sz) {
        if(req_hdr->am_hdr != &req_hdr->am_hdr_buf[0])
            MPL_free(req_hdr->am_hdr);

        req_hdr->am_hdr = MPL_malloc(am_hdr_sz);
        MPIU_Assert(req_hdr->am_hdr);
        req_hdr->am_hdr_sz = am_hdr_sz;
    }

    if(am_hdr) {
        MPIU_Memcpy(req_hdr->am_hdr, am_hdr, am_hdr_sz);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_AM_OFI_INIT_REQ);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Repost_buffer
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Repost_buffer(void         *buf,
                                                  MPID_Request *req)
{
    int           mpi_errno = MPI_SUCCESS;
    MPIDI_CH4_NMI_OFI_Am_repost_request_t *am = (MPIDI_CH4_NMI_OFI_Am_repost_request_t *)req;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_REPOST_BUFFER);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_REPOST_BUFFER);
    MPIDI_CH4_NMI_OFI_CALL_RETRY_AM(fi_recvmsg(MPIDI_CH4_NMI_OFI_EP_RX_MSG(0),
                                               &MPIDI_Global.am_msg[am->index],
                                               FI_MULTI_RECV | FI_COMPLETION), repost);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_REPOST_BUFFER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Progress_do_queue
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Progress_do_queue(void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS, ret;
    struct fi_cq_tagged_entry cq_entry;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_PROGRESS_DO_QUEUE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_PROGRESS_DO_QUEUE);

    ret = fi_cq_read(MPIDI_Global.p2p_cq, &cq_entry, 1);

    if(unlikely(ret == -FI_EAGAIN))
        goto fn_exit;

    if(ret < 0) {
        mpi_errno = MPIDI_CH4_NMI_OFI_Handle_cq_error_util(ret);
        goto fn_fail;
    }

    if(((MPIDI_Global.cq_buff_head + 1) %
        MPIDI_CH4_NMI_OFI_NUM_CQ_BUFFERED == MPIDI_Global.cq_buff_tail) ||
       !slist_empty(&MPIDI_Global.cq_buff_list)) {
        MPIDI_CH4_NMI_OFI_Cq_list_t *list_entry = (MPIDI_CH4_NMI_OFI_Cq_list_t *) MPL_malloc(sizeof(MPIDI_CH4_NMI_OFI_Cq_list_t));
        MPIU_Assert(list_entry);
        list_entry->cq_entry = cq_entry;
        slist_insert_tail(&list_entry->entry, &MPIDI_Global.cq_buff_list);
    } else {
        MPIDI_Global.cq_buffered[MPIDI_Global.cq_buff_head].cq_entry = cq_entry;
        MPIDI_Global.cq_buff_head = (MPIDI_Global.cq_buff_head + 1) % MPIDI_CH4_NMI_OFI_NUM_CQ_BUFFERED;
    }

    if((cq_entry.flags & FI_RECV) &&
       (cq_entry.flags & FI_MULTI_RECV)) {
        mpi_errno = MPIDI_CH4_NMI_OFI_Repost_buffer(cq_entry.op_context,
                                                    MPIDI_CH4_NMI_OFI_Context_to_request(cq_entry.op_context));

        if(mpi_errno) MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_PROGRESS_DO_QUEUE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Do_send_am_header
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Do_send_am_header(int                         rank,
                                                      MPID_Comm                  *comm,
                                                      uint64_t                    reply_token,
                                                      int                         handler_id,
                                                      const void                 *am_hdr,
                                                      size_t                      am_hdr_sz,
                                                      MPID_Request               *sreq,
                                                      int                         is_reply)
{
    struct iovec iov[2];
    MPIDI_CH4_NMI_OFI_Am_header_t *msg_hdr;
    int mpi_errno = MPI_SUCCESS, c, use_rank;
    MPID_Comm *use_comm;
    MPIDI_CH4_NMI_OFI_Am_reply_token_t use_token;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_SEND_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_SEND_AM_HDR);

    MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, req_hdr) = NULL;
    mpi_errno = MPIDI_CH4_NMI_OFI_Am_init_request(am_hdr, am_hdr_sz, sreq);

    if(mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIU_Assert(handler_id < (1 << MPIDI_CH4_NMI_OFI_AM_HANDLER_ID_BITS));
    MPIU_Assert(am_hdr_sz  < (1ULL << MPIDI_CH4_NMI_OFI_AM_HDR_SZ_BITS));
    msg_hdr = &MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, msg_hdr);
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz  = am_hdr_sz;
    msg_hdr->data_sz    = 0;
    msg_hdr->am_type    = MPIDI_AMTYPE_SHORT_HDR;

    if(is_reply) {
        use_token.val       = reply_token;
        use_comm            = MPIDI_CH4R_context_id_to_comm(use_token.data.context_id);
        use_rank            = use_token.data.src_rank;
        MPIU_Assert(use_token.data.context_id  < (1 << MPIDI_CH4_NMI_OFI_AM_CONTEXT_ID_BITS));
        msg_hdr->context_id = use_token.data.context_id;
        msg_hdr->src_rank   = use_comm->rank;
    } else {
        use_comm = comm;
        use_rank = rank;
        MPIU_Assert(use_comm->context_id  < (1 << MPIDI_CH4_NMI_OFI_AM_CONTEXT_ID_BITS));
        msg_hdr->context_id = use_comm->context_id;
        msg_hdr->src_rank   = use_comm->rank;
    }

    MPIU_Assert((uint64_t)use_comm->rank < (1ULL << MPIDI_CH4_NMI_OFI_AM_RANK_BITS));

    MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, clientdata).pack_buffer = NULL;
    MPIR_cc_incr(sreq->cc_ptr, &c);

    iov[0].iov_base = msg_hdr;
    iov[0].iov_len = sizeof(*msg_hdr);

    MPIU_Assert((sizeof(*msg_hdr) + am_hdr_sz) <= MPIDI_CH4_NMI_OFI_MAX_SHORT_SEND_SIZE);
    iov[1].iov_base = MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, am_hdr);
    iov[1].iov_len = am_hdr_sz;
    MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, event_id) = MPIDI_CH4_NMI_OFI_EVENT_AM_SEND;
    MPIDI_CH4_NMI_OFI_CALL_RETRY_AM(fi_sendv(MPIDI_CH4_NMI_OFI_EP_TX_MSG(0), iov, NULL, 2,
                                             MPIDI_CH4_NMI_OFI_Comm_to_phys(use_comm, use_rank, MPIDI_CH4_NMI_OFI_API_TAG),
                                             &MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, context)), sendv);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DO_SEND_AM_HDR);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Send_am_long
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Send_am_long(int           rank,
                                                 MPID_Comm    *comm,
                                                 int           handler_id,
                                                 const void   *am_hdr,
                                                 size_t        am_hdr_sz,
                                                 const void   *data,
                                                 size_t        data_sz,
                                                 MPID_Request *sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4_NMI_OFI_Am_header_t       *msg_hdr;
    MPIDI_CH4_NMI_OFI_Lmt_msg_payload_t *lmt_info;
    struct iovec              iov[3];
    uint64_t                  index;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AM_LONG);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AM_LONG);

    MPIU_Assert(handler_id       < (1    << MPIDI_CH4_NMI_OFI_AM_HANDLER_ID_BITS));
    MPIU_Assert(am_hdr_sz        < (1ULL << MPIDI_CH4_NMI_OFI_AM_HDR_SZ_BITS));
    MPIU_Assert(data_sz          < (1ULL << MPIDI_CH4_NMI_OFI_AM_DATA_SZ_BITS));
    MPIU_Assert(comm->context_id < (1    << MPIDI_CH4_NMI_OFI_AM_CONTEXT_ID_BITS));
    MPIU_Assert((uint64_t)comm->rank       < (1ULL << MPIDI_CH4_NMI_OFI_AM_RANK_BITS));

    msg_hdr             = &MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, msg_hdr);
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz  = am_hdr_sz;
    msg_hdr->data_sz    = data_sz;
    msg_hdr->am_type    = MPIDI_AMTYPE_LMT_REQ;
    msg_hdr->context_id = comm->context_id;
    msg_hdr->src_rank   = comm->rank;


    lmt_info = &MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, lmt_info);
    lmt_info->src_offset = (uint64_t) 0; /* TODO: Set to data if MR_BASIC */
    lmt_info->sreq_ptr = (uint64_t) sreq;
    /* Always allocates RMA ID from COMM_WORLD as the actual associated communicator
       is not available here */
    index = MPIDI_CH4_NMI_OFI_Index_allocator_alloc(MPIDI_CH4_NMI_OFI_COMM(MPIR_Process.comm_world).rma_id_allocator);
    MPIU_Assert((int)index < MPIDI_Global.max_huge_rmas);
    lmt_info->rma_key = index << MPIDI_Global.huge_rma_shift;

    MPIR_cc_incr(sreq->cc_ptr, &c); /* send completion */
    MPIR_cc_incr(sreq->cc_ptr, &c); /* lmt ack handler */
    MPIU_Assert((sizeof(*msg_hdr) + sizeof(*lmt_info) + am_hdr_sz) <= MPIDI_CH4_NMI_OFI_MAX_SHORT_SEND_SIZE);
    MPIDI_CH4_NMI_OFI_CALL(fi_mr_reg(MPIDI_Global.domain,
                                     data,
                                     data_sz,
                                     FI_REMOTE_READ,
                                     0ULL,
                                     lmt_info->rma_key,
                                     0ULL,
                                     &MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, lmt_mr),
                                     NULL), mr_reg);

    iov[0].iov_base = msg_hdr;
    iov[0].iov_len = sizeof(*msg_hdr);

    iov[1].iov_base = MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, am_hdr);
    iov[1].iov_len = am_hdr_sz;

    iov[2].iov_base = lmt_info;
    iov[2].iov_len = sizeof(*lmt_info);
    MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, event_id) = MPIDI_CH4_NMI_OFI_EVENT_AM_SEND;
    MPIDI_CH4_NMI_OFI_CALL_RETRY_AM(fi_sendv(MPIDI_CH4_NMI_OFI_EP_TX_MSG(0), iov, NULL, 3,
                                             MPIDI_CH4_NMI_OFI_Comm_to_phys(comm, rank, MPIDI_CH4_NMI_OFI_API_TAG),
                                             &MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, context)), sendv);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AM_LONG);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Send_am_short
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Send_am_short(int           rank,
                                                  MPID_Comm    *comm,
                                                  int           handler_id,
                                                  const void   *am_hdr,
                                                  size_t        am_hdr_sz,
                                                  const void   *data,
                                                  MPI_Count     count,
                                                  MPID_Request *sreq)
{
    int mpi_errno = MPI_SUCCESS, c;
    MPIDI_CH4_NMI_OFI_Am_header_t *msg_hdr;
    struct iovec iov[3];

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_AM_SHORT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_AM_SHORT);

    MPIU_Assert(handler_id           < (1    << MPIDI_CH4_NMI_OFI_AM_HANDLER_ID_BITS));
    MPIU_Assert(am_hdr_sz            < (1ULL << MPIDI_CH4_NMI_OFI_AM_HDR_SZ_BITS));
    MPIU_Assert((uint64_t)count      < (1ULL << MPIDI_CH4_NMI_OFI_AM_DATA_SZ_BITS));
    MPIU_Assert(comm->context_id     < (1    << MPIDI_CH4_NMI_OFI_AM_CONTEXT_ID_BITS));
    MPIU_Assert((uint64_t)comm->rank < (1ULL << MPIDI_CH4_NMI_OFI_AM_RANK_BITS));

    msg_hdr = &MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, msg_hdr);
    msg_hdr->handler_id = handler_id;
    msg_hdr->am_hdr_sz  = am_hdr_sz;
    msg_hdr->data_sz    = count;
    msg_hdr->am_type    = MPIDI_AMTYPE_SHORT;
    msg_hdr->context_id = comm->context_id;
    msg_hdr->src_rank   = comm->rank;

    iov[0].iov_base     = msg_hdr;
    iov[0].iov_len      = sizeof(*msg_hdr);

    iov[1].iov_base     = MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, am_hdr);
    iov[1].iov_len      = am_hdr_sz;

    iov[2].iov_base     = (void *) data;
    iov[2].iov_len      = count;

    MPIR_cc_incr(sreq->cc_ptr, &c);
    MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, event_id) = MPIDI_CH4_NMI_OFI_EVENT_AM_SEND;
    MPIDI_CH4_NMI_OFI_CALL_RETRY_AM(fi_sendv(MPIDI_CH4_NMI_OFI_EP_TX_MSG(0), iov, NULL, 3,
                                             MPIDI_CH4_NMI_OFI_Comm_to_phys(comm, rank, MPIDI_CH4_NMI_OFI_API_TAG),
                                             &MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, context)), sendv);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_AM_SHORT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Do_send_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NMI_OFI_Do_send_am(int           rank,
                                               MPID_Comm    *comm,
                                               uint64_t      reply_token,
                                               int           handler_id,
                                               const void   *am_hdr,
                                               size_t        am_hdr_sz,
                                               const void   *buf,
                                               size_t        count,
                                               MPI_Datatype  datatype,
                                               MPID_Request *sreq,
                                               int           is_reply)
{
    int             dt_contig, mpi_errno = MPI_SUCCESS, use_rank;
    MPID_Comm      *use_comm;
    char           *send_buf;
    MPIDI_msg_sz_t  data_sz;
    MPI_Aint        dt_true_lb, last;
    MPID_Datatype  *dt_ptr;
    MPIDI_CH4_NMI_OFI_Am_reply_token_t  use_token;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DO_SEND_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DO_SEND_AM);

    MPIDI_CH4_NMI_OFI_AMREQUEST(sreq, req_hdr) = NULL;
    mpi_errno = MPIDI_CH4_NMI_OFI_Am_init_request(am_hdr, am_hdr_sz, sreq);

    if(mpi_errno) MPIR_ERR_POP(mpi_errno);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);
    send_buf = (char *) buf + dt_true_lb;

    if(is_reply) {
        use_token.val = reply_token;
        use_comm = MPIDI_CH4R_context_id_to_comm(use_token.data.context_id);
        use_rank = use_token.data.src_rank;
    } else {
        use_comm = comm;
        use_rank = rank;
    }

    if(!dt_contig) {
        MPIDI_msg_sz_t segment_first;
        struct MPID_Segment *segment_ptr;
        segment_ptr = MPID_Segment_alloc();
        MPIR_ERR_CHKANDJUMP1(segment_ptr == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Send MPID_Segment_alloc");
        MPID_Segment_init(buf, count, datatype, segment_ptr, 0);
        segment_first = 0;
        last = data_sz;
        MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, clientdata).pack_buffer = (char *) MPL_malloc(data_sz);
        MPIR_ERR_CHKANDJUMP1(MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, clientdata).pack_buffer == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Send Pack buffer alloc");
        MPID_Segment_pack(segment_ptr, segment_first, &last, MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, clientdata).pack_buffer);
        MPID_Segment_free(segment_ptr);
        send_buf = (char *) MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, clientdata).pack_buffer;
    } else {
        MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, clientdata).pack_buffer = NULL;
    }

    mpi_errno = ((am_hdr_sz + data_sz + sizeof(MPIDI_CH4_NMI_OFI_Am_header_t)) < MPIDI_CH4_NMI_OFI_MAX_SHORT_SEND_SIZE) ?
                MPIDI_CH4_NMI_OFI_Send_am_short(use_rank, use_comm, handler_id, MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, am_hdr),
                                                am_hdr_sz, send_buf, data_sz, sreq) :
                MPIDI_CH4_NMI_OFI_Send_am_long(use_rank, use_comm, handler_id, MPIDI_CH4_NMI_OFI_AMREQUEST_HDR(sreq, am_hdr),
                                               am_hdr_sz, send_buf, data_sz, sreq);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_DO_SEND_AM);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NMI_Do_inject(int           rank,
                                          MPID_Comm    *comm,
                                          uint64_t      reply_token,
                                          int           handler_id,
                                          const void   *am_hdr,
                                          size_t        am_hdr_sz,
                                          void         *netmod_context,
                                          int           is_reply,
                                          int           use_comm_table)
{
    int mpi_errno = MPI_SUCCESS, use_rank;
    MPID_Comm *use_comm;
    MPIDI_CH4_NMI_OFI_Am_header_t msg_hdr;
    struct fi_msg msg;
    struct iovec msg_iov[2];
    MPIDI_CH4_NMI_OFI_Am_reply_token_t  use_token;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_DO_INJECT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_DO_INJECT);

    MPIU_Assert(am_hdr_sz + sizeof(msg_hdr) < MPIDI_Global.max_buffered_send);

    MPIU_Assert(handler_id       < (1    << MPIDI_CH4_NMI_OFI_AM_HANDLER_ID_BITS));
    MPIU_Assert(am_hdr_sz        < (1ULL << MPIDI_CH4_NMI_OFI_AM_HDR_SZ_BITS));

    msg_hdr.handler_id  = handler_id;
    msg_hdr.am_hdr_sz   = am_hdr_sz;
    msg_hdr.data_sz     = 0;
    msg_hdr.am_type     = MPIDI_AMTYPE_SHORT;

    if(is_reply) {
        use_token.val      = reply_token;
        use_comm           = MPIDI_CH4R_context_id_to_comm(use_token.data.context_id);
        use_rank           = use_token.data.src_rank;
        MPIU_Assert(use_token.data.context_id < (1 << MPIDI_CH4_NMI_OFI_AM_CONTEXT_ID_BITS));
        msg_hdr.context_id = use_token.data.context_id;
        msg_hdr.src_rank   = use_comm->rank;
    } else {
        use_comm           = comm;
        use_rank           = rank;
        MPIU_Assert(use_comm->context_id < (1 << MPIDI_CH4_NMI_OFI_AM_CONTEXT_ID_BITS));
        msg_hdr.context_id = use_comm->context_id;
        msg_hdr.src_rank   = use_comm->rank;
    }

    MPIU_Assert((uint64_t)use_comm->rank < (1ULL << MPIDI_CH4_NMI_OFI_AM_RANK_BITS));

    msg_iov[0].iov_base = (void *) &msg_hdr;
    msg_iov[0].iov_len  = sizeof(msg_hdr);

    msg_iov[1].iov_base = (void *) am_hdr;
    msg_iov[1].iov_len  = am_hdr_sz;

    msg.msg_iov         = &msg_iov[0];
    msg.desc            = NULL;
    msg.iov_count       = 2;
    msg.context         = NULL;
    msg.addr            = use_comm_table ?
                          MPIDI_CH4_NMI_OFI_Comm_to_phys(use_comm, use_rank, MPIDI_CH4_NMI_OFI_API_MSG):
                          MPIDI_CH4_NMI_OFI_To_phys(use_rank, MPIDI_CH4_NMI_OFI_API_MSG);

    MPIDI_CH4_NMI_OFI_CALL_RETRY_AM(fi_sendmsg(MPIDI_CH4_NMI_OFI_EP_TX_MSG(0), &msg, FI_INJECT), send);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_DO_INJECT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


static inline void MPIDI_CH4_NMI_OFI_Am_request_complete(MPID_Request *req)
{
    int count;
    MPIR_cc_decr(req->cc_ptr, &count);
    MPIU_Assert(count >= 0);

    if(count == 0) {
        MPIDI_CH4R_Request_release(req);
    }
}

#endif /*NETMOD_OFI_AM_IMPL_H_INCLUDED */
