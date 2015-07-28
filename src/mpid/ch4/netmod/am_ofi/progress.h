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
#ifndef NETMOD_AM_OFI_PROGRESS_H_INCLUDED
#define NETMOD_AM_OFI_PROGRESS_H_INCLUDED

#include "impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_short_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_short_am(MPIDI_AM_OFI_hdr_t *msg_hdr,
					       fi_addr_t source)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq;
    void *p_data;
    void *in_data;

    MPIDI_msg_sz_t  data_sz, in_data_sz;
    MPIDI_netmod_am_completion_handler_fn cmpl_handler_fn;
    struct iovec *iov;
    int i, is_contig, done, curr_len, rem, iov_len;    

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_SHORT_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_SHORT_AM);

    p_data = in_data = (char*)msg_hdr->payload + msg_hdr->am_hdr_sz;
    in_data_sz = data_sz = msg_hdr->data_sz;

    MPIDI_Global.am_handlers[msg_hdr->handler_id](
	msg_hdr->payload, msg_hdr->am_hdr_sz, (void*)source,
	&p_data, &data_sz, &is_contig, &cmpl_handler_fn, &rreq);

    if (!rreq)
	goto fn_exit;
    
    if ((!p_data || !data_sz) && cmpl_handler_fn) {
	cmpl_handler_fn(rreq);
	goto fn_exit;
    }

    if (is_contig) {
        if (in_data_sz > data_sz) {
            rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        } else {
            rreq->status.MPI_ERROR = MPI_SUCCESS;
        }

        data_sz = MIN(data_sz, in_data_sz);
        MPIU_Memcpy(p_data, in_data, data_sz);
        MPIR_STATUS_SET_COUNT(rreq->status, data_sz);
    } else {
        done = 0;
        rem = in_data_sz;
        iov = (struct iovec*) p_data;
        iov_len = data_sz;
        for (i = 0; i < iov_len && rem > 0; i++) {
            curr_len = MIN(rem, iov[i].iov_len);
            MPIU_Memcpy(iov[i].iov_base, (char *)in_data + done, curr_len);
            rem -= curr_len;
            done += curr_len;
        }
        if (rem) {
            rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        } else {
            rreq->status.MPI_ERROR = MPI_SUCCESS;
        }
        MPIR_STATUS_SET_COUNT(rreq->status, done);
    }

    if (cmpl_handler_fn)
	cmpl_handler_fn(rreq);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_SHORT_AM);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_short_am_hdr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_short_am_hdr(MPIDI_AM_OFI_hdr_t *msg_hdr,
						   fi_addr_t source)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_netmod_am_completion_handler_fn cmpl_handler_fn;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_SHORT_AM_HDR);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_SHORT_AM_HDR);

    MPIDI_Global.am_handlers[msg_hdr->handler_id](
	msg_hdr->payload, msg_hdr->am_hdr_sz, (void*)source,
	NULL, NULL, NULL, NULL, NULL);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_SHORT_AM_HDR);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_do_rdma_read
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_do_rdma_read(void *dst, uint64_t src, size_t data_sz,
                                            fi_addr_t source, MPID_Request *rreq)
{
    int mpi_errno = MPI_SUCCESS;
    int done = 0, curr_len, rem = 0;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DO_RDMA_READ);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DO_RDMA_READ);

    rem = data_sz;
    while (done != data_sz) {
	curr_len = MIN(rem, MPIDI_Global.max_send);
	FI_RC_RETRY(fi_read(MPIDI_Global.ep, (char *)dst + done,
                            curr_len, NULL, source, src + done,
			    MPIDI_Global.lkey, &AMREQ_OFI(rreq, context)), read);
	done += curr_len;
	rem -= curr_len;
    }
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_DO_RDMA_READ);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_long_am
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_long_am(MPIDI_AM_OFI_hdr_t *msg_hdr,
					      fi_addr_t source)
{
    int mpi_errno = MPI_SUCCESS, is_contig = 0;
    MPID_Request *rreq;
    void *p_data, *data;
    size_t data_sz, rem, done, curr_len, in_data_sz;
    MPIDI_OFI_lmt_msg_pyld_t *lmt_msg;
    MPIDI_netmod_am_completion_handler_fn cmpl_handler_fn;
    int num_reads, i, iov_len;
    struct iovec *iov;
    
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_LONG_AM);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_LONG_AM);

    in_data_sz = data_sz = msg_hdr->data_sz;
    MPIDI_Global.am_handlers[msg_hdr->handler_id](
	msg_hdr->payload, msg_hdr->am_hdr_sz, (void*)source,
	&p_data, &data_sz, &is_contig, &cmpl_handler_fn, &rreq);
    AMREQ_OFI(rreq, cmpl_handler_fn) = cmpl_handler_fn;
    
    if (!rreq)
	goto fn_exit;
    
    if ((!p_data || !data_sz) && cmpl_handler_fn) {
	cmpl_handler_fn(rreq);
	goto fn_exit;
    }

    lmt_msg = (MPIDI_OFI_lmt_msg_pyld_t*)((char*)msg_hdr->payload + msg_hdr->am_hdr_sz);
    AMREQ_OFI(rreq, lmt_info) = *lmt_msg;

    if (is_contig) {
        if (in_data_sz > data_sz) {
            rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        } else {
            rreq->status.MPI_ERROR = MPI_SUCCESS;
        }

        data_sz = MIN(data_sz, in_data_sz);
        AMREQ_OFI(rreq, lmt_cntr) = 
            ((data_sz - 1) / MPIDI_Global.max_send) + 1;
        MPIDI_netmod_do_rdma_read(p_data, lmt_msg->src_offset, data_sz,
                                  source, rreq);
        MPIR_STATUS_SET_COUNT(rreq->status, data_sz);
    } else {
        done = 0;
        rem = in_data_sz;
        iov = (struct iovec*) p_data;
        iov_len = data_sz;

        /* FIXME: optimize iov processing part */

        /* set lmt counter */
        AMREQ_OFI(rreq, lmt_cntr) = 0;
        for (i = 0; i < iov_len && rem > 0; i++) {
            curr_len = MIN(rem, iov[i].iov_len);
            num_reads = ((curr_len - 1) / MPIDI_Global.max_send) + 1;
            AMREQ_OFI(rreq, lmt_cntr) += num_reads;
            rem -= curr_len;
        }

        done = 0;
        rem = in_data_sz;
        for (i = 0; i < iov_len && rem > 0; i++) {
            curr_len = MIN(rem, iov[i].iov_len);
            MPIDI_netmod_do_rdma_read(iov[i].iov_base, lmt_msg->src_offset + done, curr_len, source, rreq);
            rem -= curr_len;
            done += curr_len;
        }
        if (rem) {
            rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        } else {
            rreq->status.MPI_ERROR = MPI_SUCCESS;
        }
        MPIR_STATUS_SET_COUNT(rreq->status, done);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_LONG_AM);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_ack_msg
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_ack_msg(MPIDI_AM_OFI_hdr_t *msg_hdr,
					      fi_addr_t source)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq;
    MPIDI_OFI_Ack_msg_pyld_t *ack_msg;
    int handler_id;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_ACK_MSG);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_ACK_MSG);

    ack_msg = (MPIDI_OFI_Ack_msg_pyld_t*)msg_hdr->payload;
    sreq = (MPID_Request *)ack_msg->sreq_ptr;

    handler_id = AMREQ_OFI(sreq, msg_hdr).handler_id;
    MPI_RC_POP(MPIDI_Global.send_cmpl_handlers[handler_id](sreq));

    if (handler_id == MPIDI_AMTYPE_LMT_ACK && 
	AMREQ_OFI(sreq, pack_buffer)) {
	MPIU_Free(AMREQ_OFI(sreq, pack_buffer));
    }
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_SHORT_AM);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_AMOFI_IS_CTRL_MSG(uint8_t msg_type)
{
    switch(msg_type) {
    case MPIDI_AMTYPE_ACK:
    case MPIDI_AMTYPE_LMT_REQ:
	return 1;
    default:
	return 0;
    }
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_send_completion
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_send_completion(
    struct fi_cq_data_entry *cq_entry)
{
    int mpi_errno = MPI_SUCCESS, count;
    MPID_Request *sreq;
    MPIDI_OFIAMReq_t *ofi_req;
    MPIDI_AM_OFI_hdr_t *msg_hdr;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_SEND_COMPLETION);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_SEND_COMPLETION);

    ofi_req = container_of(cq_entry->op_context, MPIDI_OFIAMReq_t, context);
    msg_hdr = &ofi_req->msg_hdr;

    if (MPIDI_AMOFI_IS_CTRL_MSG(msg_hdr->am_type))
	goto fn_exit;

    sreq = container_of(ofi_req, MPID_Request, dev.ch4u.netmod_am);
    if (AMREQ_OFI(sreq, pack_buffer)) {
	MPIU_Free(AMREQ_OFI(sreq, pack_buffer));
    }

    MPI_RC_POP(MPIDI_Global.send_cmpl_handlers[msg_hdr->handler_id](sreq));
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_SEND_COMPLETION);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_recv_completion
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_recv_completion(
    struct fi_cq_data_entry *cq_entry, fi_addr_t source,
    void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_AM_OFI_hdr_t *am_hdr;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_RECV_COMPLETION);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_RECV_COMPLETION);

    am_hdr = (MPIDI_AM_OFI_hdr_t*) cq_entry->buf;
    switch(am_hdr->am_type) {
    case MPIDI_AMTYPE_SHORT_HDR:
	MPI_RC_POP(MPIDI_netmod_handle_short_am_hdr(am_hdr, source));
	break;

    case MPIDI_AMTYPE_SHORT:
	MPI_RC_POP(MPIDI_netmod_handle_short_am(am_hdr, source));
	break;

    case MPIDI_AMTYPE_LMT_REQ:
	MPI_RC_POP(MPIDI_netmod_handle_long_am(am_hdr, source));
	break;

    case MPIDI_AMTYPE_LMT_ACK:
    case MPIDI_AMTYPE_ACK:
	MPI_RC_POP(MPIDI_netmod_handle_ack_msg(am_hdr, source));
	break;

    default:
	MPIU_Assert(0);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_RECV_COMPLETION);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_dispatch_ack
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_dispatch_ack(fi_addr_t source,
					    MPIDI_OFIAMReq_t *ofi_req,
					    void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_OFI_Ack_msg_t msg;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_DISPATCH_ACK);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_DISPATCH_ACK);

    msg.hdr.am_hdr_sz = sizeof(msg.pyld);
    msg.hdr.data_sz = 0;
    msg.hdr.am_type = MPIDI_AMTYPE_ACK;
    msg.pyld.sreq_ptr = ofi_req->lmt_info.sreq_ptr;

    FI_RC_RETRY(fi_inject(MPIDI_Global.ep, &msg, sizeof(msg), source), inject);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_DISPATCH_ACK);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_handle_read_completion
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_handle_read_completion(
    struct fi_cq_data_entry *cq_entry, fi_addr_t source,
    void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *rreq;
    MPIDI_OFIAMReq_t *ofi_req;
    int dt_contig;
    MPI_Aint        dt_true_lb;
    MPID_Datatype  *dt_ptr;
    MPI_Aint last;
    size_t data_sz, dt_sz;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_HANDLE_READ_COMPLETION);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_HANDLE_READ_COMPLETION);

    ofi_req = container_of(cq_entry->op_context, MPIDI_OFIAMReq_t, context);
    ofi_req->lmt_cntr--;
    if (ofi_req->lmt_cntr)
	goto fn_exit;

    rreq = container_of(ofi_req, MPID_Request, dev.ch4u.netmod_am);
    MPI_RC_POP(MPIDI_netmod_dispatch_ack(source, ofi_req, netmod_context));    
    ofi_req->cmpl_handler_fn(rreq);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_HANDLE_READ_COMPLETION);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_repost_buffer
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_repost_buffer(void *buf, void *netmod_context)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_REPOST_BUFFER);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_REPOST_BUFFER);

    FI_RC_RETRY(fi_recvmsg(MPIDI_Global.ep, (struct fi_msg *)buf,
			   FI_MULTI_RECV), repost);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_REPOST_BUFFER);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_progress
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_progress(void *netmod_context, int blocking)
{
    int mpi_errno = MPI_SUCCESS, found = 0, ret;
    struct fi_cq_data_entry cq_entry;
    struct fi_cq_err_entry cq_err_entry;
    fi_addr_t source;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_PROGRESS);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_PROGRESS);

    do {
	ret = fi_cq_readfrom(MPIDI_Global.am_cq, &cq_entry, 1, &source);
	if (ret == -FI_EAGAIN)
	    continue;

	if (ret < 0) {
	    fi_cq_readerr(MPIDI_Global.am_cq, &cq_err_entry, 0);
	    fprintf(stderr, "fi_cq_read failed with error: %s\n",
		    fi_strerror(cq_err_entry.err));
	    goto fn_fail;
	}

	found = 1;
	if (cq_entry.flags & FI_SEND) {
	    MPI_RC_POP(MPIDI_netmod_handle_send_completion(&cq_entry));

	} else if (cq_entry.flags & FI_RECV) {
	    MPI_RC_POP(MPIDI_netmod_handle_recv_completion(
			   &cq_entry, source, netmod_context));

	    if (cq_entry.flags & FI_MULTI_RECV) {
		MPI_RC_POP(MPIDI_netmod_repost_buffer(
			       cq_entry.op_context,
			       netmod_context));
	    }

	} else if (cq_entry.flags & FI_READ) {
	    MPI_RC_POP(MPIDI_netmod_handle_read_completion(
			   &cq_entry, source, netmod_context));

	} else {
	    MPIU_Assert(0);
	}
    } while(blocking && !found);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_PROGRESS);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_netmod_progress_test(void)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_progress_poke(void)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline void MPIDI_netmod_progress_start(MPID_Progress_state * state)
{
    MPIU_Assert(0);
    return;
}

static inline void MPIDI_netmod_progress_end(MPID_Progress_state * state)
{
    MPIU_Assert(0);
    return;
}

static inline int MPIDI_netmod_progress_wait(MPID_Progress_state * state)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_progress_register(int (*progress_fn) (int *), int *id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_progress_deregister(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}
static inline int MPIDI_netmod_progress_activate(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_progress_deactivate(int id)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

#endif /* NETMOD_AM_OFI_PROGRESS_H_INCLUDED */
