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
#ifndef MPIDCH4U_RECVQ_H_INCLUDED
#define MPIDCH4U_RECVQ_H_INCLUDED

#include <mpidimpl.h>
#include "mpl_utlist.h"
#include "ch4_impl.h"

#ifdef MPIDI_CH4U_USE_PER_COMM_QUEUE

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_enqueue_posted
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4I_enqueue_posted(MPID_Request * req, MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4_ENQUEUE_POSTED);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_ENQUEUE_POSTED);
    MPL_DL_APPEND(*list, &req->dev.ch4.ch4u.rreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_ENQUEUE_POSTED);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_enqueue_unexp
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4I_enqueue_unexp(MPID_Request * req, MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4_ENQUEUE_UNEXP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_ENQUEUE_UNEXP);
    MPL_DL_APPEND(*list, &req->dev.ch4.ch4u.rreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_ENQUEUE_UNEXP);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_delete_unexp
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4I_delete_unexp(MPID_Request * req, MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4_DELETE_UNEXP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_DELETE_UNEXP);
    MPL_DL_DELETE(*list, &req->dev.ch4.ch4u.rreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_DELETE_UNEXP);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_dequeue_unexp_strict
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ MPID_Request *MPIDI_CH4I_dequeue_unexp_strict(uint64_t tag, uint64_t ignore,
                                                             MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPIDI_CH4U_Dev_rreq_t *curr, *tmp;
    MPIDI_CH4U_Devreq_t *dev_req;
    MPID_Request *req = NULL;
    MPIDI_STATE_DECL(MPID_STATE_CH4_DEQUEUE_UNEXP_STRICT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_DEQUEUE_UNEXP_STRICT);

    MPL_DL_FOREACH_SAFE(*list, curr, tmp) {
        dev_req = container_of(curr, MPIDI_CH4U_Devreq_t, rreq);
        if (!(dev_req->status & MPIDI_CH4U_REQ_BUSY) && ((tag & ~ignore) == (dev_req->tag & ~ignore))) {
            req = container_of(curr, MPID_Request, dev.ch4.ch4u.rreq);
            MPL_DL_DELETE(*list, curr);
            break;
        }
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_DEQUEUE_UNEXP_STRICT);
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_dequeue_unexp
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ MPID_Request *MPIDI_CH4I_dequeue_unexp(uint64_t tag, uint64_t ignore,
                                                      MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPIDI_CH4U_Dev_rreq_t *curr, *tmp;
    MPIDI_CH4U_Devreq_t *dev_req;
    MPID_Request *req = NULL;
    MPIDI_STATE_DECL(MPID_STATE_CH4_DEQUEUE_UNEXP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_DEQUEUE_UNEXP);

    MPL_DL_FOREACH_SAFE(*list, curr, tmp) {
        dev_req = container_of(curr, MPIDI_CH4U_Devreq_t, rreq);
        if ((tag & ~ignore) == (dev_req->tag & ~ignore)) {
            req = container_of(curr, MPID_Request, dev.ch4.ch4u.rreq);
            MPL_DL_DELETE(*list, curr);
            break;
        }
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_DEQUEUE_UNEXP);
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_find_unexp
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ MPID_Request *MPIDI_CH4I_find_unexp(uint64_t tag, uint64_t ignore,
                                                   MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPIDI_CH4U_Dev_rreq_t *curr, *tmp;
    MPIDI_CH4U_Devreq_t *dev_req;
    MPID_Request *req = NULL;
    MPIDI_STATE_DECL(MPID_STATE_CH4_FIND_UNEXP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_FIND_UNEXP);

    MPL_DL_FOREACH_SAFE(*list, curr, tmp) {
        dev_req = container_of(curr, MPIDI_CH4U_Devreq_t, rreq);
        if ((tag & ~ignore) == (dev_req->tag & ~ignore)) {
            req = container_of(curr, MPID_Request, dev.ch4.ch4u.rreq);
            break;
        }
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_FIND_UNEXP);
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_dequeue_posted
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ MPID_Request *MPIDI_CH4I_dequeue_posted(uint64_t tag, MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPID_Request *req = NULL;
    MPIDI_CH4U_Devreq_t *dev_req;
    MPIDI_CH4U_Dev_rreq_t *curr, *tmp;
    MPIDI_STATE_DECL(MPID_STATE_CH4_DEQUEUE_POSTED);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_DEQUEUE_POSTED);

    MPL_DL_FOREACH_SAFE(*list, curr, tmp) {
        dev_req = container_of(curr, MPIDI_CH4U_Devreq_t, rreq);
        if ((tag & ~dev_req->rreq.ignore) == (dev_req->tag & ~dev_req->rreq.ignore)) {
            req = container_of(curr, MPID_Request, dev.ch4.ch4u.rreq);
            MPL_DL_DELETE(*list, curr);
            break;
        }
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_DEQUEUE_POSTED);
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_delete_posted
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4I_delete_posted(MPIDI_CH4U_Dev_rreq_t * req, MPIDI_CH4U_Dev_rreq_t ** list)
{
    int found = 0;
    MPIDI_CH4U_Dev_rreq_t *curr, *tmp;
    MPIDI_STATE_DECL(MPID_STATE_CH4_DELETE_POSTED);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_DELETE_POSTED);
    MPL_DL_FOREACH_SAFE(*list, curr, tmp) {
        if (curr == req) {
            MPL_DL_DELETE(*list, curr);
            found = 1;
            break;
        }
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_DELETE_POSTED);
    return found;
}

#else /* #ifdef MPIDI_CH4U_USE_PER_COMM_QUEUE */

/* Use global queue */

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_enqueue_posted
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4I_enqueue_posted(MPID_Request * req, MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4_ENQUEUE_POSTED);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_ENQUEUE_POSTED);
    MPL_DL_APPEND(MPIDI_CH4_Global.posted_list, &req->dev.ch4.ch4u.rreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_ENQUEUE_POSTED);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_enqueue_unexp
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4I_enqueue_unexp(MPID_Request * req, MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4_ENQUEUE_UNEXP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_ENQUEUE_UNEXP);
    MPL_DL_APPEND(MPIDI_CH4_Global.unexp_list, &req->dev.ch4.ch4u.rreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_ENQUEUE_UNEXP);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_delete_unexp
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void MPIDI_CH4I_delete_unexp(MPID_Request * req, MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4_DELETE_UNEXP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_DELETE_UNEXP);
    MPL_DL_DELETE(MPIDI_CH4_Global.unexp_list, &req->dev.ch4.ch4u.rreq);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_DELETE_UNEXP);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_dequeue_unexp_strict
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ MPID_Request *MPIDI_CH4I_dequeue_unexp_strict(uint64_t tag, uint64_t ignore,
                                                             MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPIDI_CH4U_Dev_rreq_t *curr, *tmp;
    MPIDI_CH4U_Devreq_t *dev_req;
    MPID_Request *req = NULL;
    MPIDI_STATE_DECL(MPID_STATE_CH4_DEQUEUE_UNEXP_STRICT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_DEQUEUE_UNEXP_STRICT);

    MPL_DL_FOREACH_SAFE(MPIDI_CH4_Global.unexp_list, curr, tmp) {
        dev_req = container_of(curr, MPIDI_CH4U_Devreq_t, rreq);
        if (!(dev_req->status & MPIDI_CH4U_REQ_BUSY) && ((tag & ~ignore) ==
                                                         (dev_req->tag & ~ignore))) {
            req = container_of(curr, MPID_Request, dev.ch4.ch4u.rreq);
            MPL_DL_DELETE(MPIDI_CH4_Global.unexp_list, curr);
            break;
        }
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_DEQUEUE_UNEXP_STRICT);
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_dequeue_unexp
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ MPID_Request *MPIDI_CH4I_dequeue_unexp(uint64_t tag, uint64_t ignore,
                                                      MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPIDI_CH4U_Dev_rreq_t *curr, *tmp;
    MPIDI_CH4U_Devreq_t *dev_req;
    MPID_Request *req = NULL;
    MPIDI_STATE_DECL(MPID_STATE_CH4_DEQUEUE_UNEXP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_DEQUEUE_UNEXP);

    MPL_DL_FOREACH_SAFE(MPIDI_CH4_Global.unexp_list, curr, tmp) {
        dev_req = container_of(curr, MPIDI_CH4U_Devreq_t, rreq);
        if ((tag & ~ignore) == (dev_req->tag & ~ignore)) {
            req = container_of(curr, MPID_Request, dev.ch4.ch4u.rreq);
            MPL_DL_DELETE(MPIDI_CH4_Global.unexp_list, curr);
            break;
        }
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_DEQUEUE_UNEXP);
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_find_unexp
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ MPID_Request *MPIDI_CH4I_find_unexp(uint64_t tag, uint64_t ignore,
                                                   MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPIDI_CH4U_Dev_rreq_t *curr, *tmp;
    MPIDI_CH4U_Devreq_t *dev_req;
    MPID_Request *req = NULL;
    MPIDI_STATE_DECL(MPID_STATE_CH4_FIND_UNEXP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_FIND_UNEXP);

    MPL_DL_FOREACH_SAFE(MPIDI_CH4_Global.unexp_list, curr, tmp) {
        dev_req = container_of(curr, MPIDI_CH4U_Devreq_t, rreq);
        if ((tag & ~ignore) == (dev_req->tag & ~ignore)) {
            req = container_of(curr, MPID_Request, dev.ch4.ch4u.rreq);
            break;
        }
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_FIND_UNEXP);
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_dequeue_posted
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ MPID_Request *MPIDI_CH4I_dequeue_posted(uint64_t tag, MPIDI_CH4U_Dev_rreq_t ** list)
{
    MPID_Request *req = NULL;
    MPIDI_CH4U_Devreq_t *dev_req;
    MPIDI_CH4U_Dev_rreq_t *curr, *tmp;
    MPIDI_STATE_DECL(MPID_STATE_CH4_DEQUEUE_POSTED);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_DEQUEUE_POSTED);

    MPL_DL_FOREACH_SAFE(MPIDI_CH4_Global.posted_list, curr, tmp) {
        dev_req = container_of(curr, MPIDI_CH4U_Devreq_t, rreq);
        if ((tag & ~dev_req->rreq.ignore) == (dev_req->tag & ~dev_req->rreq.ignore)) {
            req = container_of(curr, MPID_Request, dev.ch4.ch4u.rreq);
            MPL_DL_DELETE(MPIDI_CH4_Global.posted_list, curr);
            break;
        }
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_DEQUEUE_POSTED);
    return req;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4I_delete_posted
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_CH4I_delete_posted(MPIDI_CH4U_Dev_rreq_t * req, MPIDI_CH4U_Dev_rreq_t ** list)
{
    int found = 0;
    MPIDI_CH4U_Dev_rreq_t *curr, *tmp;
    MPIDI_STATE_DECL(MPID_STATE_CH4_DELETE_POSTED);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_DELETE_POSTED);
    MPL_DL_FOREACH_SAFE(MPIDI_CH4_Global.posted_list, curr, tmp) {
        if (curr == req) {
            MPL_DL_DELETE(MPIDI_CH4_Global.posted_list, curr);
            found = 1;
            break;
        }
    }
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_DELETE_POSTED);
    return found;
}

#endif /* MPIDI_CH4U_USE_PER_COMM_QUEUE */

#endif /* MPIDCH4U_RECVQ_H_INCLUDED */