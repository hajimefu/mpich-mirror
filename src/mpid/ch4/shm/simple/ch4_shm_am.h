/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef SHM_AM_H_INCLUDED
#define SHM_AM_H_INCLUDED

#include "ch4_shm_impl.h"

static inline int MPIDI_CH4_SHM_reg_hdr_handler(int handler_id,
                                                MPIDI_CH4_SHM_am_origin_handler_fn origin_handler_fn,
                                                MPIDI_CH4_SHM_am_target_handler_fn target_handler_fn)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_send_am_hdr(int rank,
                                            MPID_Comm *comm,
                                            int handler_id,
                                            const void *am_hdr,
                                            size_t am_hdr_sz, MPID_Request *sreq, void *shm_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_send_am(int rank,
                                        MPID_Comm *comm,
                                        int handler_id,
                                        const void *am_hdr,
                                        size_t am_hdr_sz,
                                        const void *data,
                                        MPI_Count count,
                                        MPI_Datatype datatype, MPID_Request *sreq, void *shm_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_send_amv(int rank,
                                         MPID_Comm *comm,
                                         int handler_id,
                                         struct iovec *am_hdr,
                                         size_t iov_len,
                                         const void *data,
                                         MPI_Count count,
                                         MPI_Datatype datatype, MPID_Request *sreq, void *shm_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_send_am_hdr_reply(void *reply_token,
                                                  int handler_id,
                                                  const void *am_hdr,
                                                  size_t am_hdr_sz, MPID_Request *sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_send_am_reply(void *reply_token,
                                              int handler_id,
                                              const void *am_hdr,
                                              size_t am_hdr_sz,
                                              const void *data,
                                              MPI_Count count,
                                              MPI_Datatype datatype, MPID_Request *sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_send_amv_reply(void *reply_token,
                                               int handler_id,
                                               struct iovec *am_hdr,
                                               size_t iov_len,
                                               const void *data,
                                               MPI_Count count,
                                               MPI_Datatype datatype, MPID_Request *sreq)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline size_t MPIDI_CH4_SHM_am_hdr_max_sz(void)
{
    MPIU_Assert(0);
    return 0;
}

static inline int MPIDI_CH4_SHM_inject_am_hdr(int rank,
                                              MPID_Comm *comm,
                                              int handler_id,
                                              const void *am_hdr, size_t am_hdr_sz, void *shm_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_inject_am(int rank,
                                          MPID_Comm *comm,
                                          int handler_id,
                                          const void *am_hdr,
                                          size_t am_hdr_sz,
                                          const void *data,
                                          MPI_Count count, MPI_Datatype datatype, void *shm_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_inject_amv(int rank,
                                           MPID_Comm *comm,
                                           int handler_id,
                                           struct iovec *am_hdr,
                                           size_t iov_len,
                                           const void *data,
                                           MPI_Count count, MPI_Datatype datatype, void *shm_context)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_inject_am_hdr_reply(void *reply_token,
                                                    int handler_id,
                                                    const void *am_hdr, size_t am_hdr_sz)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_inject_am_reply(void *reply_token,
                                                int handler_id,
                                                const void *am_hdr,
                                                size_t am_hdr_sz,
                                                const void *data,
                                                MPI_Count count, MPI_Datatype datatype)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline int MPIDI_CH4_SHM_inject_amv_reply(void *reply_token,
                                                 int handler_id,
                                                 struct iovec *am_hdr,
                                                 size_t iov_len,
                                                 const void *data,
                                                 MPI_Count count, MPI_Datatype datatype)
{
    MPIU_Assert(0);
    return MPI_SUCCESS;
}

static inline size_t MPIDI_CH4_SHM_am_inject_max_sz(void)
{
    MPIU_Assert(0);
    return 0;
}

#endif /* SHM_AM_H_INCLUDED */
