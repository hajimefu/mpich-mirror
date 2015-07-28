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
#ifndef NETMOD_AMSTUB_AM_H_INCLUDED
#define NETMOD_AMSTUB_AM_H_INCLUDED

#include "impl.h"

static inline int MPIDI_netmod_reg_hdr_handler(int handler_id,
    MPIDI_netmod_am_origin_handler_fn origin_handler_fn,
    MPIDI_netmod_am_target_handler_fn target_handler_fn)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_send_am_hdr(int rank,
                                           MPID_Comm * comm,
                                           int handler_id,
                                           const void *am_hdr,
                                           size_t am_hdr_sz,
                                           MPID_Request * sreq, void *netmod_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_send_am(int rank,
                                       MPID_Comm * comm,
                                       int handler_id,
                                       const void *am_hdr,
                                       size_t am_hdr_sz,
                                       const void *data,
				       MPI_Count count,
				       MPI_Datatype datatype,
				       MPID_Request * sreq, void *netmod_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_send_amv(int rank,
                                        MPID_Comm * comm,
                                        int handler_id,
                                        struct iovec *am_hdr,
                                        size_t iov_len,
                                        const void *data,
					MPI_Count count,
					MPI_Datatype datatype,
					MPID_Request * sreq, void *netmod_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_send_am_hdr_reply(void *reply_token,
                                                 int handler_id,
                                                 const void *am_hdr,
                                                 size_t am_hdr_sz, MPID_Request * sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_send_am_reply(void *reply_token,
                                             int handler_id,
                                             const void *am_hdr,
                                             size_t am_hdr_sz,
                                             const void *data,
					     MPI_Count count,
					     MPI_Datatype datatype,
					     MPID_Request * sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_send_amv_reply(void *reply_token,
                                              int handler_id,
                                              struct iovec *am_hdr,
                                              size_t iov_len,
                                              const void *data,
					      MPI_Count count,
					      MPI_Datatype datatype,
					      MPID_Request * sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline size_t MPIDI_netmod_am_hdr_max_sz(void)
{
    MPIU_Assert(0);
    return 0;
}

static inline int MPIDI_netmod_inject_am_hdr(int rank,
					     MPID_Comm * comm,
					     int handler_id,
					     const void *am_hdr,
					     size_t am_hdr_sz,
					     void *netmod_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_inject_am_hdr_reply(void *reply_token,
						   int handler_id,
						   const void *am_hdr,
						   size_t am_hdr_sz)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline size_t MPIDI_netmod_am_inject_max_sz(void)
{
    MPIU_Assert(0);
    return 0;
}


#endif /* NETMOD_AMSTUB_AM_H_INCLUDED */
