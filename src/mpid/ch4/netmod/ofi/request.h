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
#ifndef NETMOD_OFI_REQUEST_H_INCLUDED
#define NETMOD_OFI_REQUEST_H_INCLUDED

#include "impl.h"

static inline void MPIDI_CH4_NM_native_request_release(MPID_Request * req)
{
    MPIDI_CH4_NMI_OFI_request_release(req);
}

static inline void MPIDI_CH4_NM_am_request_release(MPID_Request * req)
{
    MPIDI_CH4_NMI_OFI_AM_request_release(req);
}

static inline void MPIDI_CH4_NM_am_request_init(MPID_Request *req)
{
    AMREQ_OFI(req, req_hdr) = NULL;
}

#endif /* NETMOD_OFI_REQUEST_H_INCLUDED */
