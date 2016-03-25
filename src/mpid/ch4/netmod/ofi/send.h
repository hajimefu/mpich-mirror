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
#ifndef NETMOD_OFI_SEND_H_INCLUDED
#define NETMOD_OFI_SEND_H_INCLUDED

#include "impl.h"
#include <mpibsend.h>
#include <../mpi/pt2pt/bsendutil.h>

#define MPIDI_CH4_NMI_OFI_SENDPARAMS const void *buf,int count,MPI_Datatype datatype, \
    int rank,int tag,MPID_Comm *comm,                               \
    int context_offset,MPID_Request **request

#define MPIDI_CH4_NMI_OFI_SENDARGS buf,count,datatype,rank,tag, \
                 comm,context_offset,request

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Send_lightweight
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NMI_OFI_Send_lightweight(const void     *buf,
                                                         size_t  data_sz,
                                                         int             rank,
                                                         int             tag,
                                                         MPID_Comm      *comm,
                                                         int             context_offset)
{
    int mpi_errno = MPI_SUCCESS;
    uint64_t match_bits;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_LIGHTWEIGHT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_LIGHTWEIGHT);
    match_bits = MPIDI_CH4_NMI_OFI_Init_sendtag(comm->context_id + context_offset, tag, 0);
    MPIDI_CH4_NMI_OFI_CALL_RETRY(fi_tinjectdata(MPIDI_CH4_NMI_OFI_EP_TX_TAG(0), buf, data_sz, comm->rank,
                                                MPIDI_CH4_NMI_OFI_Comm_to_phys(comm, rank, MPIDI_CH4_NMI_OFI_API_TAG),
                                                match_bits), tinjectdata);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_LIGHTWEIGHT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Send_lightweight_request
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NMI_OFI_Send_lightweight_request(const void     *buf,
                                                                 size_t  data_sz,
                                                                 int             rank,
                                                                 int             tag,
                                                                 MPID_Comm      *comm,
                                                                 int             context_offset,
                                                                 MPID_Request  **request)
{
    int mpi_errno = MPI_SUCCESS;
    uint64_t match_bits;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_LIGHTWEIGHT_REQUEST);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_LIGHTWEIGHT_REQUEST);
    MPID_Request *r;
    MPIDI_CH4_NMI_OFI_SEND_REQUEST_CREATE_LW(r);
    *request = r;
    match_bits = MPIDI_CH4_NMI_OFI_Init_sendtag(comm->context_id + context_offset, tag, 0);
    MPIDI_CH4_NMI_OFI_CALL_RETRY(fi_tinjectdata(MPIDI_CH4_NMI_OFI_EP_TX_TAG(0), buf, data_sz, comm->rank,
                                            MPIDI_CH4_NMI_OFI_Comm_to_phys(comm, rank, MPIDI_CH4_NMI_OFI_API_TAG),
                                            match_bits), tinjectdata);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_LIGHTWEIGHT_REQUEST);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Send_normal
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NMI_OFI_Send_normal(MPIDI_CH4_NMI_OFI_SENDPARAMS,
                                                    int              dt_contig,
                                                    size_t   data_sz,
                                                    MPID_Datatype   *dt_ptr,
                                                    MPI_Aint         dt_true_lb,
                                                    uint64_t         type)
{
    int mpi_errno = MPI_SUCCESS;
    MPID_Request *sreq = NULL;
    MPI_Aint last;
    char *send_buf;
    uint64_t match_bits;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_NORMAL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_NORMAL);

    MPIDI_CH4_NMI_OFI_REQUEST_CREATE(sreq);
    sreq->kind = MPID_REQUEST_SEND;
    *request = sreq;
    match_bits = MPIDI_CH4_NMI_OFI_Init_sendtag(comm->context_id + context_offset, tag, type);
    MPIDI_CH4_NMI_OFI_REQUEST(sreq, event_id) = MPIDI_CH4_NMI_OFI_EVENT_SEND;
    MPIDI_CH4_NMI_OFI_REQUEST(sreq, datatype) = datatype;
    dtype_add_ref_if_not_builtin(datatype);

    if(type == MPIDI_CH4_NMI_OFI_SYNC_SEND) {        /* Branch should compile out */
        int c = 1;
        uint64_t ssend_match, ssend_mask;
        MPIDI_CH4_NMI_OFI_Ssendack_request_t *ackreq;
        MPIDI_CH4_NMI_OFI_SSEND_ACKREQUEST_CREATE(ackreq);
        ackreq->event_id = MPIDI_CH4_NMI_OFI_EVENT_SSEND_ACK;
        ackreq->signal_req = sreq;
        MPIR_cc_incr(sreq->cc_ptr, &c);
        ssend_match = MPIDI_CH4_NMI_OFI_Init_recvtag(&ssend_mask, comm->context_id + context_offset, tag);
        ssend_match |= MPIDI_CH4_NMI_OFI_SYNC_SEND_ACK;
        MPIDI_CH4_NMI_OFI_CALL_RETRY(fi_trecv(MPIDI_CH4_NMI_OFI_EP_RX_TAG(0),      /* endpoint    */
                                              NULL,              /* recvbuf     */
                                              0,                 /* data sz     */
                                              NULL,              /* memregion descr  */
                                              MPIDI_CH4_NMI_OFI_Comm_to_phys(comm, rank, MPIDI_CH4_NMI_OFI_API_TAG),  /* remote proc */
                                              ssend_match,       /* match bits  */
                                              0ULL,              /* mask bits   */
                                              (void *) &(ackreq->context)), trecvsync);
    }

    send_buf = (char *) buf + dt_true_lb;

    if(!dt_contig) {
        size_t segment_first;
        segment_first = 0;
        last = data_sz;
        MPIDI_CH4_NMI_OFI_REQUEST(sreq, noncontig) = (MPIDI_CH4_NMI_OFI_Noncontig_t *) MPL_malloc(data_sz+sizeof(MPID_Segment));
        MPIR_ERR_CHKANDJUMP1(MPIDI_CH4_NMI_OFI_REQUEST(sreq, noncontig) == NULL, mpi_errno,
                             MPI_ERR_OTHER, "**nomem", "**nomem %s", "Send Pack buffer alloc");
        MPID_Segment_init(buf, count, datatype, &MPIDI_CH4_NMI_OFI_REQUEST(sreq,noncontig->segment), 0);
        MPID_Segment_pack(&MPIDI_CH4_NMI_OFI_REQUEST(sreq,noncontig->segment), segment_first, &last,
                          MPIDI_CH4_NMI_OFI_REQUEST(sreq, noncontig->pack_buffer));
        send_buf = MPIDI_CH4_NMI_OFI_REQUEST(sreq, noncontig->pack_buffer);
    }
    else
        MPIDI_CH4_NMI_OFI_REQUEST(sreq, noncontig) = NULL;

    if(data_sz <= MPIDI_Global.max_buffered_send) {
        MPIDI_CH4_NMI_OFI_CALL_RETRY(fi_tinjectdata(MPIDI_CH4_NMI_OFI_EP_TX_TAG(0), send_buf, data_sz, comm->rank,
                                                MPIDI_CH4_NMI_OFI_Comm_to_phys(comm, rank, MPIDI_CH4_NMI_OFI_API_TAG), match_bits), tinjectdata);
        MPIDI_CH4_NMI_OFI_Send_event(NULL, sreq);
    } else if(data_sz <= MPIDI_Global.max_send)
        MPIDI_CH4_NMI_OFI_CALL_RETRY(fi_tsenddata(MPIDI_CH4_NMI_OFI_EP_TX_TAG(0), /* Destination endpoint */
                                                  send_buf, /* Buffer being sent */
                                                  data_sz, /* Buffer size */
                                                  NULL, /* Data buffer memory descriptor */
                                                  comm->rank, /* Data field - contains rank */
                                                  MPIDI_CH4_NMI_OFI_Comm_to_phys(comm, rank, MPIDI_CH4_NMI_OFI_API_TAG), /* Destination address */
                                                  match_bits, /* Match bits for the destination to use */
                                                  (void *) &(MPIDI_CH4_NMI_OFI_REQUEST(sreq, context))), /* Context pointer */
                                     tsenddata);
    else if(unlikely(1)) {
        MPIDI_CH4_NMI_OFI_Send_control_t ctrl;
        int c;
        uint64_t rma_key;
        MPIDI_CH4_NMI_OFI_Huge_counter_t *cntr;
        void *ptr;
        c = 1;
        MPIDI_CH4_NMI_OFI_REQUEST(sreq, event_id) = MPIDI_CH4_NMI_OFI_EVENT_SEND_HUGE;
        MPIR_cc_incr(sreq->cc_ptr, &c);
        ptr = MPIDI_CH4_NMI_OFI_Map_lookup(MPIDI_CH4_NMI_OFI_COMM(comm).huge_send_counters, rank);

        MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_FI_MUTEX);

        if(ptr == MPIDI_CH4_NMI_OFI_MAP_NOT_FOUND) {
            ptr = MPL_malloc(sizeof(MPIDI_CH4_NMI_OFI_Huge_counter_t));
            cntr = (MPIDI_CH4_NMI_OFI_Huge_counter_t *) ptr;
            cntr->outstanding = 0;
            cntr->counter = 0;
            MPIDI_CH4_NMI_OFI_Map_set(MPIDI_CH4_NMI_OFI_COMM(comm).huge_send_counters, rank, ptr);
        }

        cntr = (MPIDI_CH4_NMI_OFI_Huge_counter_t *) ptr;

        ctrl.rma_key = MPIDI_CH4_NMI_OFI_Index_allocator_alloc(MPIDI_CH4_NMI_OFI_COMM(comm).rma_id_allocator);
        MPIU_Assert(ctrl.rma_key < MPIDI_Global.max_huge_rmas);
        rma_key  = ctrl.rma_key<<MPIDI_Global.huge_rma_shift;
        MPIDI_CH4_NMI_OFI_CALL_NOLOCK(fi_mr_reg(MPIDI_Global.domain,     /* In:  Domain Object       */
                                                send_buf,                /* In:  Lower memory address*/
                                                data_sz,                 /* In:  Length              */
                                                FI_REMOTE_READ,          /* In:  Expose MR for read  */
                                                0ULL,                    /* In:  offset(not used)    */
                                                rma_key,                 /* In:  requested key       */
                                                0ULL,                    /* In:  flags               */
                                                &cntr->mr,               /* Out: memregion object    */
                                                NULL), mr_reg);          /* In:  context             */

        cntr->outstanding++;
        cntr->counter++;
        MPIU_Assert(cntr->outstanding != USHRT_MAX);
        MPIU_Assert(cntr->counter != USHRT_MAX);
        MPIDI_CH4_NMI_OFI_REQUEST(sreq, util_comm) = comm;
        MPIDI_CH4_NMI_OFI_REQUEST(sreq, util_id)   = rank;
        MPIDI_CH4_NMI_OFI_CALL_RETRY_NOLOCK(fi_tsenddata(MPIDI_CH4_NMI_OFI_EP_TX_TAG(0), send_buf,
                                                     MPIDI_Global.max_send,
                                                     NULL,
                                                     comm->rank,
                                                     MPIDI_CH4_NMI_OFI_Comm_to_phys(comm, rank,MPIDI_CH4_NMI_OFI_API_TAG),
                                                     match_bits, (void *) &(MPIDI_CH4_NMI_OFI_REQUEST(sreq, context))),
                                            tsenddata);
        ctrl.type = MPIDI_CH4_NMI_OFI_CTRL_HUGE;
        ctrl.seqno = cntr->counter - 1;
        MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Do_control_send(&ctrl, send_buf, data_sz, rank, comm, sreq, FALSE));
        MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_FI_MUTEX);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_NORMAL);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NMI_OFI_Send(MPIDI_CH4_NMI_OFI_SENDPARAMS, int noreq, uint64_t syncflag)
{
    int dt_contig, mpi_errno;
    size_t data_sz;
    MPI_Aint dt_true_lb;
    MPID_Datatype *dt_ptr;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NM_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NM_SEND);

    if(unlikely(rank == MPI_PROC_NULL)) {
        mpi_errno = MPI_SUCCESS;

        if(!noreq) {
            MPIDI_CH4_NMI_OFI_REQUEST_CREATE((*request));
            (*request)->kind = MPID_REQUEST_SEND;
            MPIDI_CH4U_request_complete((*request));
        }

        goto fn_exit;
    }

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    if(likely(!syncflag && dt_contig && (data_sz <= MPIDI_Global.max_buffered_send)))
        if(noreq)
            mpi_errno = MPIDI_CH4_NMI_OFI_Send_lightweight((char *) buf + dt_true_lb, data_sz,
                                                           rank, tag, comm, context_offset);
        else
            mpi_errno = MPIDI_CH4_NMI_OFI_Send_lightweight_request((char *) buf + dt_true_lb, data_sz,
                                                                   rank, tag, comm, context_offset,
                                                                   request);
    else
        mpi_errno = MPIDI_CH4_NMI_OFI_Send_normal(buf, count, datatype, rank, tag, comm,
                                                  context_offset, request, dt_contig,
                                                  data_sz, dt_ptr, dt_true_lb, syncflag);

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NM_SEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NMI_OFI_Persistent_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NMI_OFI_Persistent_send(MPIDI_CH4_NMI_OFI_SENDPARAMS)
{
    MPID_Request *sreq;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_NM_PSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_NM_PSEND);

    MPIDI_CH4_NMI_OFI_REQUEST_CREATE(sreq);
    *request = sreq;

    MPIR_Comm_add_ref(comm);
    sreq->kind = MPID_PREQUEST_SEND;
    sreq->comm = comm;
    MPIDI_CH4_NMI_OFI_REQUEST(sreq, util.persist.buf)   = (void *) buf;
    MPIDI_CH4_NMI_OFI_REQUEST(sreq, util.persist.count) = count;
    MPIDI_CH4_NMI_OFI_REQUEST(sreq, datatype)           = datatype;
    MPIDI_CH4_NMI_OFI_REQUEST(sreq, util.persist.rank)  = rank;
    MPIDI_CH4_NMI_OFI_REQUEST(sreq, util.persist.tag)   = tag;
    MPIDI_CH4_NMI_OFI_REQUEST(sreq, util_comm)          = comm;
    MPIDI_CH4_NMI_OFI_REQUEST(sreq, util_id)            = comm->context_id + context_offset;
    sreq->partner_request             = NULL;
    MPIDI_CH4U_request_complete(sreq);

    if(HANDLE_GET_KIND(datatype) != HANDLE_KIND_BUILTIN) {
        MPID_Datatype *dt_ptr;
        MPID_Datatype_get_ptr(datatype, dt_ptr);
        MPID_Datatype_add_ref(dt_ptr);
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_NM_PSEND);
    return MPI_SUCCESS;
}

#define STARTALL_CASE(CASELABEL,FUNC,CONTEXTID) \
  case CASELABEL:                               \
  {                                             \
    rc = FUNC(MPIDI_CH4_NMI_OFI_REQUEST(preq,util.persist.buf),              \
              MPIDI_CH4_NMI_OFI_REQUEST(preq,util.persist.count),            \
              MPIDI_CH4_NMI_OFI_REQUEST(preq,datatype),           \
              MPIDI_CH4_NMI_OFI_REQUEST(preq,util.persist.rank),             \
              MPIDI_CH4_NMI_OFI_REQUEST(preq,util.persist.tag),              \
              preq->comm,                       \
              MPIDI_CH4_NMI_OFI_REQUEST(preq,util_id) -           \
              CONTEXTID,                        \
              &preq->partner_request);          \
    break;                                      \
  }

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_send(MPIDI_CH4_NMI_OFI_SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND);
    mpi_errno = MPIDI_CH4_NMI_OFI_Send(MPIDI_CH4_NMI_OFI_SENDARGS, 1, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_rsend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_rsend(MPIDI_CH4_NMI_OFI_SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RSEND);
    mpi_errno = MPIDI_CH4_NMI_OFI_Send(MPIDI_CH4_NMI_OFI_SENDARGS, 1, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RSEND);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_irsend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_irsend(MPIDI_CH4_NMI_OFI_SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_IRSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_IRSEND);
    mpi_errno = MPIDI_CH4_NMI_OFI_Send(MPIDI_CH4_NMI_OFI_SENDARGS, 0, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_IRSEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_ssend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_ssend(MPIDI_CH4_NMI_OFI_SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SSEND);
    mpi_errno = MPIDI_CH4_NMI_OFI_Send(MPIDI_CH4_NMI_OFI_SENDARGS, 0, MPIDI_CH4_NMI_OFI_SYNC_SEND);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SSEND);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_isend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_isend(MPIDI_CH4_NMI_OFI_SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_ISEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_ISEND);
    mpi_errno = MPIDI_CH4_NMI_OFI_Send(MPIDI_CH4_NMI_OFI_SENDARGS, 0, 0ULL);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_ISEND);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_issend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_issend(MPIDI_CH4_NMI_OFI_SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_ISSEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_ISSEND);
    mpi_errno = MPIDI_CH4_NMI_OFI_Send(MPIDI_CH4_NMI_OFI_SENDARGS, 0, MPIDI_CH4_NMI_OFI_SYNC_SEND);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_ISSEND);
    return mpi_errno;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_startall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_startall(int count, MPID_Request *requests[])
{
    int rc = MPI_SUCCESS, i;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_STARTALL);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_STARTALL);

    for(i = 0; i < count; i++) {
        MPID_Request *const preq = requests[i];

        switch(MPIDI_CH4_NMI_OFI_REQUEST(preq, util.persist.type)) {
#ifdef MPIDI_BUILD_CH4_SHM
                STARTALL_CASE(MPIDI_PTYPE_RECV, MPIDI_CH4_NM_irecv, preq->comm->recvcontext_id);
#else
                STARTALL_CASE(MPIDI_PTYPE_RECV, MPIDI_Irecv, preq->comm->recvcontext_id);
#endif

#ifdef MPIDI_BUILD_CH4_SHM
                STARTALL_CASE(MPIDI_PTYPE_SEND, MPIDI_CH4_NM_isend, preq->comm->context_id);
#else
                STARTALL_CASE(MPIDI_PTYPE_SEND, MPIDI_Isend, preq->comm->context_id);
#endif
                STARTALL_CASE(MPIDI_PTYPE_SSEND, MPIDI_Issend, preq->comm->context_id);

            case MPIDI_PTYPE_BSEND: {
                MPI_Request sreq_handle;
                rc = MPIR_Ibsend_impl(MPIDI_CH4_NMI_OFI_REQUEST(preq, util.persist.buf),
                                      MPIDI_CH4_NMI_OFI_REQUEST(preq, util.persist.count),
                                      MPIDI_CH4_NMI_OFI_REQUEST(preq, datatype),
                                      MPIDI_CH4_NMI_OFI_REQUEST(preq, util.persist.rank),
                                      MPIDI_CH4_NMI_OFI_REQUEST(preq, util.persist.tag),
                                      preq->comm,
                                      &sreq_handle);

                if(rc == MPI_SUCCESS)
                    MPID_Request_get_ptr(sreq_handle, preq->partner_request);

                break;
            }

            default:
                rc = MPIR_Err_create_code(MPI_SUCCESS, MPIR_ERR_FATAL, __FUNCTION__,
                                          __LINE__, MPI_ERR_INTERN, "**ch3|badreqtype",
                                          "**ch3|badreqtype %d", MPIDI_CH4_NMI_OFI_REQUEST(preq, util.persist.type));
        }

        if(rc == MPI_SUCCESS) {
            preq->status.MPI_ERROR = MPI_SUCCESS;

            if(MPIDI_CH4_NMI_OFI_REQUEST(preq, util.persist.type) == MPIDI_PTYPE_BSEND) {
                preq->cc_ptr = &preq->cc;
                MPIR_cc_set(&preq->cc, 0);
            } else
                preq->cc_ptr = &preq->partner_request->cc;
        } else {
            preq->partner_request = NULL;
            preq->status.MPI_ERROR = rc;
            preq->cc_ptr = &preq->cc;
            MPIR_cc_set(&preq->cc, 0);
        }
    }

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_STARTALL);
    return rc;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_send_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_send_init(MPIDI_CH4_NMI_OFI_SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SEND_INIT);
    mpi_errno = MPIDI_CH4_NMI_OFI_Persistent_send(MPIDI_CH4_NMI_OFI_SENDARGS);
    MPIDI_CH4_NMI_OFI_REQUEST((*request), util.persist.type) = MPIDI_PTYPE_SEND;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SEND_INIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_ssend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_ssend_init(MPIDI_CH4_NMI_OFI_SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_SSEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_SSEND_INIT);
    mpi_errno = MPIDI_CH4_NMI_OFI_Persistent_send(MPIDI_CH4_NMI_OFI_SENDARGS);
    MPIDI_CH4_NMI_OFI_REQUEST((*request), util.persist.type) = MPIDI_PTYPE_SSEND;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_SSEND_INIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_bsend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_bsend_init(MPIDI_CH4_NMI_OFI_SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_BSEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_BSEND_INIT);
    mpi_errno = MPIDI_CH4_NMI_OFI_Persistent_send(MPIDI_CH4_NMI_OFI_SENDARGS);
    MPIDI_CH4_NMI_OFI_REQUEST((*request), util.persist.type) = MPIDI_PTYPE_BSEND;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_BSEND_INIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_rsend_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_rsend_init(MPIDI_CH4_NMI_OFI_SENDPARAMS)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_RSEND_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_RSEND_INIT);
    mpi_errno = MPIDI_CH4_NMI_OFI_Persistent_send(MPIDI_CH4_NMI_OFI_SENDARGS);
    MPIDI_CH4_NMI_OFI_REQUEST((*request), util.persist.type) = MPIDI_PTYPE_SEND;
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_RSEND_INIT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_cancel_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__ALWAYS_INLINE__ int MPIDI_CH4_NM_cancel_send(MPID_Request *sreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_CANCEL_SEND);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_CANCEL_SEND);
    /* Sends cannot be cancelled */

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_CANCEL_SEND);
    return mpi_errno;
}

#endif /* NETMOD_OFI_SEND_H_INCLUDED */
