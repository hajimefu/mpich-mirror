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
#ifndef CH4_REQUEST_H_INCLUDED
#define CH4_REQUEST_H_INCLUDED

#include "ch4_impl.h"
#include "ch4r_buf.h"

__CH4_INLINE__ int MPIDI_Request_is_anysource(MPIR_Request * req)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ int MPIDI_Request_is_pending_failure(MPIR_Request * req)
{
    MPIR_Assert(0);
    return MPI_SUCCESS;
}

__CH4_INLINE__ void MPIDI_Request_set_completed(MPIR_Request * req)
{
    MPIR_cc_set(&req->cc, 0);
    return;
}

__CH4_INLINE__ void MPIDI_Request_add_ref(MPIR_Request * req)
{
    MPIR_Request_add_ref(req);
    return;
}

__CH4_INLINE__ void MPIDI_Request_release_ref(MPIR_Request * req)
{
    int inuse;
    MPIR_Request_release_ref(req, &inuse);
    return;
}

/* These request functions should be called by the MPI layer only
   since they only do base initialization of the request object.
   A few notes:

   It is each layer's responsibility to initialize a request
   properly.

   The CH4I_request functions are even more bare bones.
   They create request objects that are not useable by the
   lower layers until further initialization takes place.

   CH4R_request_xxx functions can be used to create and destroy
   request objects at any CH4 layer, including shmmod and netmod.
   These functions create and initialize a base request with
   the appropriate "above device" fields initialized, and any
   required CH4 layer fields initialized.

   The net/shm mods can upcall to CH4R to create a request, or
   they can iniitalize their own requests internally, but note
   that all the fields from the upper layers must be initialized
   properly.

   Note that the request_release function is used by the MPI
   layer to release the ref on a request object.  It is important
   for the netmods to release any memory pointed to by the request
   when the internal completion counters hits zero, NOT when the
   ref hits zero or there will be a memory leak. The generic
   release function will not release any memory pointed to by
   the request because it does not know about the internals of
   the ch4r/netmod/shmmod fields of the request.
*/
#undef FUNCNAME
#define FUNCNAME MPIDI_request_complete
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Request_complete(MPIR_Request * req)
{
    int incomplete;
    MPIR_cc_decr(req->cc_ptr, &incomplete);
    MPIR_Request_free(req);
    return MPI_SUCCESS;
}

#endif /* CH4_REQUEST_H_INCLUDED */
