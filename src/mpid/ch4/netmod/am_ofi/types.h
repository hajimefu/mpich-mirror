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
#ifndef NETMOD_AM_OFI_TYPES_H_INCLUDED
#define NETMOD_AM_OFI_TYPES_H_INCLUDED

#include "../ofi/types.h"
#include <mpidch4u.h>

static inline int MPIDI_netmod_am_ofi_init_req(const void *am_hdr,
                                               size_t am_hdr_sz,
                                               MPID_Request *sreq,
                                               int is_reply);

static inline void MPIDI_netmod_am_ofi_clear_req(MPID_Request *sreq);
static inline void MPIDI_netmod_am_ofi_req_complete(MPID_Request *req);

EXTERN_C_BEGIN
#define AMREQ_OFI(req,field) ((req)->dev.ch4.ch4u.netmod_am.am_ofi.field)
#define AMREQ_OFI_HDR(req,field) ((req)->dev.ch4.ch4u.netmod_am.am_ofi.req_hdr->field)

#endif /* NETMOD_AM_OFI_TYPES_H_INCLUDED */
