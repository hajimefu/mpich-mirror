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
#ifndef SHM_COLL_H_INCLUDED
#define SHM_COLL_H_INCLUDED

#include "ch4_shm_impl.h"
#include "ch4_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_barrier
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_barrier(MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_bcast
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_bcast(void *buffer, int count, MPI_Datatype datatype,
                                         int root, MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_allreduce
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
                                             MPI_Op op, MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_allgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                             void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                             MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_allgatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                              void *recvbuf, const int *recvcounts, const int *displs,
                                              MPI_Datatype recvtype, MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_gather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                          void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                          int root, MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_gatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                           void *recvbuf, const int *recvcounts, const int *displs,
                                           MPI_Datatype recvtype, int root, MPID_Comm *comm_ptr,
                                           MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_scatter
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                           void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                           int root, MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_scatterv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_scatterv(const void *sendbuf, const int *sendcounts,
                                            const int *displs, MPI_Datatype sendtype,
                                            void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                            int root, MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_alltoall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                            void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                            MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_alltoallv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_alltoallv(const void *sendbuf, const int *sendcounts,
                                             const int *sdispls, MPI_Datatype sendtype,
                                             void *recvbuf, const int *recvcounts,
                                             const int *rdispls, MPI_Datatype recvtype,
                                             MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);
    return mpi_errno;
}

#define FUNCNAME MPIDI_CH4_SHMI_STUB_alltoallw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_alltoallw(const void *sendbuf, const int sendcounts[],
                                         const int sdispls[], const MPI_Datatype sendtypes[],
                                         void *recvbuf, const int recvcounts[],
                                         const int rdispls[], const MPI_Datatype recvtypes[],
                                         MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIU_Assert(0);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_reduce
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_reduce(const void *sendbuf, void *recvbuf, int count,
                                          MPI_Datatype datatype, MPI_Op op, int root,
                                          MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_reduce_scatter
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_reduce_scatter(const void *sendbuf, void *recvbuf,
                                              const int recvcounts[], MPI_Datatype datatype,
                                              MPI_Op op, MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIU_Assert(0);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_reduce_scatter_block
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_reduce_scatter_block(const void *sendbuf, void *recvbuf,
                                                    int recvcount, MPI_Datatype datatype,
                                                    MPI_Op op, MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIU_Assert(0);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_scan
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_scan(const void *sendbuf, void *recvbuf, int count,
                                        MPI_Datatype datatype, MPI_Op op, MPID_Comm *comm_ptr,
                                        MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_exscan
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_exscan(const void *sendbuf, void *recvbuf, int count,
                                          MPI_Datatype datatype, MPI_Op op, MPID_Comm *comm_ptr,
                                          MPIR_Errflag_t *errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_neighbor_allgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_neighbor_allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                                      void *recvbuf, int recvcount, MPI_Datatype recvtype, MPID_Comm *comm_ptr)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_neighbor_allgatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_neighbor_allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                                       void *recvbuf, const int recvcounts[], const int displs[],
                                                       MPI_Datatype recvtype, MPID_Comm *comm_ptr)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_neighbor_alltoall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_neighbor_alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                                     void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                                     MPID_Comm *comm_ptr)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_neighbor_alltoallv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_neighbor_alltoallv(const void *sendbuf, const int sendcounts[], const int sdispls[],
                                                      MPI_Datatype sendtype, void *recvbuf, const int recvcounts[],
                                                      const int rdispls[], MPI_Datatype recvtype, MPID_Comm *comm_ptr)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_neighbor_alltoallw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_neighbor_alltoallw(const void *sendbuf, const int sendcounts[],
                                                      const MPI_Aint sdispls[], const MPI_Datatype sendtypes[],
                                                      void *recvbuf, const int recvcounts[],
                                                      const MPI_Aint rdispls[], const MPI_Datatype recvtypes[],
                                                      MPID_Comm *comm_ptr)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_ineighbor_allgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_ineighbor_allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                                      void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                                      MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_ineighbor_allgatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_ineighbor_allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                                       void *recvbuf, const int recvcounts[], const int displs[],
                                                       MPI_Datatype recvtype, MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_ineighbor_alltoall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_ineighbor_alltoall(const void *sendbuf, int sendcount,
                                                      MPI_Datatype sendtype, void *recvbuf,
                                                      int recvcount, MPI_Datatype recvtype,
                                                      MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_ineighbor_alltoallv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_ineighbor_alltoallv(const void *sendbuf, const int sendcounts[], const int sdispls[],
                                                      MPI_Datatype sendtype, void *recvbuf, const int recvcounts[],
                                                      const int rdispls[], MPI_Datatype recvtype, MPID_Comm *comm_ptr,
                                                      MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_ineighbor_alltoallw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_ineighbor_alltoallw(const void *sendbuf, const int sendcounts[],
                                                       const MPI_Aint sdispls[], const MPI_Datatype sendtypes[],
                                                       void *recvbuf, const int recvcounts[],
                                                       const MPI_Aint rdispls[], const MPI_Datatype recvtypes[],
                                                       MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_ibarrier
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_ibarrier(MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_Ibarrier(comm_ptr, req);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_ibcast
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_ibcast(void *buffer, int count, MPI_Datatype datatype,
                                          int root, MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_iallgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_iallgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                              void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                              MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_iallgatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_iallgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                               void *recvbuf, const int *recvcounts, const int *displs,
                                               MPI_Datatype recvtype, MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_ialltoall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_ialltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                             void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                             MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_ialltoallv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_ialltoallv(const void *sendbuf, const int *sendcounts,
                                              const int *sdispls, MPI_Datatype sendtype,
                                              void *recvbuf, const int *recvcounts,
                                              const int *rdispls, MPI_Datatype recvtype,
                                              MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_SHMI_STUB_ialltoallw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_SHMI_STUB_ialltoallw(const void *sendbuf, const int *sendcounts,
                                              const int *sdispls, const MPI_Datatype sendtypes[],
                                              void *recvbuf, const int *recvcounts,
                                              const int *rdispls, const MPI_Datatype recvtypes[],
                                              MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIU_Assert(0);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_iexscan
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_iexscan(const void *sendbuf, void *recvbuf, int count,
                                           MPI_Datatype datatype, MPI_Op op, MPID_Comm *comm_ptr,
                                           MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_igather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_igather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                           void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                           int root, MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_igatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_igatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                            void *recvbuf, const int *recvcounts, const int *displs,
                                            MPI_Datatype recvtype, int root, MPID_Comm *comm_ptr,
                                            MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_ireduce_scatter_block
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_ireduce_scatter_block(const void *sendbuf, void *recvbuf,
                                                         int recvcount, MPI_Datatype datatype,
                                                         MPI_Op op, MPID_Comm *comm_ptr,
                                                         MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_ireduce_scatter
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_ireduce_scatter(const void *sendbuf, void *recvbuf,
                                                   const int recvcounts[], MPI_Datatype datatype,
                                                   MPI_Op op, MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_ireduce
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_ireduce(const void *sendbuf, void *recvbuf, int count,
                                           MPI_Datatype datatype, MPI_Op op, int root,
                                           MPID_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHMI_STUB_iscan
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHMI_STUB_iscan(const void *sendbuf, void *recvbuf, int count,
                                         MPI_Datatype datatype, MPI_Op op, MPID_Comm *comm_ptr,
                                         MPI_Request * req)
{
    int mpi_errno = MPI_SUCCESS;

    MPIU_Assert(0);

    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_SHMI_STUB_iscatter
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_SHMI_STUB_iscatter(const void * sendbuf, int sendcount,
                                        MPI_Datatype sendtype, void * recvbuf,
                                        int recvcount, MPI_Datatype recvtype,
                                        int root, MPID_Comm *comm, MPI_Request * request)
{
    int mpi_errno;
    MPIU_Assert(0);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_SHMI_STUB_iscatterv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_SHMI_STUB_iscatterv(const void * sendbuf, const int * sendcounts,
                                         const int * displs, MPI_Datatype sendtype,
                                         void * recvbuf, int recvcount,
                                         MPI_Datatype recvtype, int root,
                                         MPID_Comm *comm, MPI_Request * request)
{
    int mpi_errno;
    MPIU_Assert(0);
    return mpi_errno;
}

#endif
