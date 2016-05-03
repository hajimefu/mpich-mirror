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
#ifndef SHM_COLL_H_INCLUDED
#define SHM_COLL_H_INCLUDED

#include "impl.h"
#include "ch4_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_barrier
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_barrier(MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_BARRIER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_BARRIER);

    mpi_errno = MPIR_Barrier(comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_BARRIER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_bcast
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_bcast(void *buffer, int count, MPI_Datatype datatype,
                                         int root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_BCAST);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_BCAST);

    mpi_errno = MPIR_Bcast( buffer, count, datatype, root, comm_ptr, errflag );

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_BCAST);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_allreduce
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_allreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
                                             MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_ALLREDUCE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_ALLREDUCE);

    mpi_errno = MPIR_Allreduce(sendbuf, recvbuf, count, datatype, op, comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_ALLREDUCE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_allgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                             void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                             MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_ALLGATHER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_ALLGATHER);

    mpi_errno = MPIR_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype,
                               comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_ALLGATHER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_allgatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                              void *recvbuf, const int *recvcounts, const int *displs,
                                              MPI_Datatype recvtype, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_ALLGATHERV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_ALLGATHERV);

    mpi_errno = MPIR_Allgatherv(sendbuf, sendcount, sendtype,
                                recvbuf, recvcounts, displs, recvtype,
                                comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_ALLGATHERV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_gather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                          void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                          int root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_GATHER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_GATHER);

    mpi_errno = MPIR_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                            recvtype, root, comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_GATHER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_gatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                           void *recvbuf, const int *recvcounts, const int *displs,
                                           MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
                                           MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_GATHERV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_GATHERV);

    mpi_errno = MPIR_Gatherv(sendbuf, sendcount, sendtype,
                             recvbuf, recvcounts, displs, recvtype,
                             root, comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_GATHERV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_scatter
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                           void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                           int root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_SCATTER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_SCATTER);

    mpi_errno = MPIR_Scatter(sendbuf, sendcount, sendtype,
                             recvbuf, recvcount, recvtype, root,
                             comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_SCATTER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_scatterv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_scatterv(const void *sendbuf, const int *sendcounts,
                                            const int *displs, MPI_Datatype sendtype,
                                            void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                            int root, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_SCATTERV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_SCATTERV);

    mpi_errno = MPIR_Scatterv(sendbuf, sendcounts, displs,
                              sendtype, recvbuf, recvcount,
                              recvtype, root, comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_SCATTERV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_alltoall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                            void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                            MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_ALLTOALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_ALLTOALL);

    mpi_errno = MPIR_Alltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                              recvtype, comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_ALLTOALL);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_alltoallv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_alltoallv(const void *sendbuf, const int *sendcounts,
                                             const int *sdispls, MPI_Datatype sendtype,
                                             void *recvbuf, const int *recvcounts,
                                             const int *rdispls, MPI_Datatype recvtype,
                                             MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_ALLTOALLV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_ALLTOALLV);

    mpi_errno = MPIR_Alltoallv(sendbuf, sendcounts, sdispls,
                               sendtype, recvbuf, recvcounts,
                               rdispls, recvtype, comm_ptr, errflag);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_ALLTOALLV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_alltoallw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_alltoallw(const void *sendbuf, const int sendcounts[],
                                         const int sdispls[], const MPI_Datatype sendtypes[],
                                         void *recvbuf, const int recvcounts[],
                                         const int rdispls[], const MPI_Datatype recvtypes[],
                                         MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_ALLTOALLW);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_ALLTOALLW);

    mpi_errno = MPIR_Alltoallw(sendbuf, sendcounts, sdispls,
                               sendtypes, recvbuf, recvcounts,
                               rdispls, recvtypes, comm_ptr, errflag);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_ALLTOALLW);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_reduce
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_reduce(const void *sendbuf, void *recvbuf, int count,
                                          MPI_Datatype datatype, MPI_Op op, int root,
                                          MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_REDUCE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_REDUCE);

    mpi_errno = MPIR_Reduce(sendbuf, recvbuf, count, datatype,
                            op, root, comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_REDUCE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_reduce_scatter
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_reduce_scatter(const void *sendbuf, void *recvbuf,
                                              const int recvcounts[], MPI_Datatype datatype,
                                              MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_REDUCE_SCATTER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_REDUCE_SCATTER);

    mpi_errno = MPIR_Reduce_scatter(sendbuf, recvbuf, recvcounts, datatype, op,
            comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_REDUCE_SCATTER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_reduce_scatter_block
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_reduce_scatter_block(const void *sendbuf, void *recvbuf,
                                                    int recvcount, MPI_Datatype datatype,
                                                    MPI_Op op, MPIR_Comm *comm_ptr, MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_REDUCE_SCATTER_BLOCK);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_REDUCE_SCATTER_BLOCK);

    mpi_errno = MPIR_Reduce_scatter_block(sendbuf, recvbuf, recvcount,
            datatype, op, comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_REDUCE_SCATTER_BLOCK);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_scan
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_scan(const void *sendbuf, void *recvbuf, int count,
                                        MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr,
                                        MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_SCAN);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_SCAN);

    mpi_errno = MPIR_Scan(sendbuf, recvbuf, count, datatype,
                          op, comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_SCAN);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_exscan
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_exscan(const void *sendbuf, void *recvbuf, int count,
                                          MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr,
                                          MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_EXSCAN);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_EXSCAN);

    mpi_errno = MPIR_Exscan(sendbuf, recvbuf, count, datatype,
                            op, comm_ptr, errflag);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_EXSCAN);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_neighbor_allgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_neighbor_allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                                   void *recvbuf, int recvcount, MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                                                   MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_NEIGHBOR_ALLGATHER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_NEIGHBOR_ALLGATHER);

    mpi_errno = MPIR_Neighbor_allgather_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm_ptr);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_NEIGHBOR_ALLGATHER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_neighbor_allgatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_neighbor_allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                                       void *recvbuf, const int recvcounts[], const int displs[],
                                                    MPI_Datatype recvtype, MPIR_Comm *comm_ptr,MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_NEIGHBOR_ALLGATHERV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_NEIGHBOR_ALLGATHERV);

    mpi_errno = MPIR_Neighbor_allgatherv_impl(sendbuf, sendcount, sendtype,
                                         recvbuf, recvcounts, displs, recvtype,
                                         comm_ptr);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_NEIGHBOR_ALLGATHERV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_neighbor_alltoall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_neighbor_alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                                     void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                                  MPIR_Comm *comm_ptr,MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_NEIGHBOR_ALLTOALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_NEIGHBOR_ALLTOALL);

    mpi_errno = MPIR_Neighbor_alltoall_impl(sendbuf, sendcount, sendtype,
                                       recvbuf, recvcount, recvtype,
                                       comm_ptr);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_NEIGHBOR_ALLTOALL);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_neighbor_alltoallv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_neighbor_alltoallv(const void *sendbuf, const int sendcounts[], const int sdispls[],
                                                      MPI_Datatype sendtype, void *recvbuf, const int recvcounts[],
                                                   const int rdispls[], MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                                                   MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_NEIGHBOR_ALLTOALLV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_NEIGHBOR_ALLTOALLV);

    mpi_errno = MPIR_Neighbor_alltoallv_impl(sendbuf, sendcounts, sdispls, sendtype,
                                                recvbuf, recvcounts, rdispls, recvtype,
                                                comm_ptr);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_NEIGHBOR_ALLTOALLV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_neighbor_alltoallw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_neighbor_alltoallw(const void *sendbuf, const int sendcounts[],
                                                      const MPI_Aint sdispls[], const MPI_Datatype sendtypes[],
                                                      void *recvbuf, const int recvcounts[],
                                                      const MPI_Aint rdispls[], const MPI_Datatype recvtypes[],
                                                   MPIR_Comm *comm_ptr,MPIR_Errflag_t *errflag)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_NEIGHBOR_ALLTOALLW);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_NEIGHBOR_ALLTOALLW);

    mpi_errno = MPIR_Neighbor_alltoallw_impl(sendbuf, sendcounts, sdispls, sendtypes,
                                        recvbuf, recvcounts, rdispls, recvtypes,
                                        comm_ptr);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_NEIGHBOR_ALLTOALLW);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ineighbor_allgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ineighbor_allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                                      void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                                      MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_INEIGHBOR_ALLGATHER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_INEIGHBOR_ALLGATHER);

    mpi_errno = MPIR_Ineighbor_allgather_impl(sendbuf, sendcount, sendtype,
                                         recvbuf, recvcount, recvtype,
                                         comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_INEIGHBOR_ALLGATHER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ineighbor_allgatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ineighbor_allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                                       void *recvbuf, const int recvcounts[], const int displs[],
                                                       MPI_Datatype recvtype, MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_INEIGHBOR_ALLGATHERV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_INEIGHBOR_ALLGATHERV);

    mpi_errno = MPIR_Ineighbor_allgatherv_impl(sendbuf, sendcount, sendtype,
                                         recvbuf, recvcounts, displs, recvtype,
                                         comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_INEIGHBOR_ALLGATHERV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ineighbor_alltoall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ineighbor_alltoall(const void *sendbuf, int sendcount,
                                                      MPI_Datatype sendtype, void *recvbuf,
                                                      int recvcount, MPI_Datatype recvtype,
                                                      MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_INEIGHBOR_ALLTOALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_INEIGHBOR_ALLTOALL);

    mpi_errno = MPIR_Ineighbor_alltoall_impl(sendbuf, sendcount, sendtype,
                                       recvbuf, recvcount, recvtype,
                                       comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_INEIGHBOR_ALLTOALL);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ineighbor_alltoallv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ineighbor_alltoallv(const void *sendbuf, const int sendcounts[], const int sdispls[],
                                                      MPI_Datatype sendtype, void *recvbuf, const int recvcounts[],
                                                      const int rdispls[], MPI_Datatype recvtype, MPIR_Comm *comm_ptr,
                                                      MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_INEIGHBOR_ALLTOALLV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_INEIGHBOR_ALLTOALLV);

    mpi_errno = MPIR_Ineighbor_alltoallv_impl(sendbuf, sendcounts, sdispls, sendtype,
                                         recvbuf, recvcounts, rdispls, recvtype,
                                         comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_INEIGHBOR_ALLTOALLV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ineighbor_alltoallw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ineighbor_alltoallw(const void *sendbuf, const int sendcounts[],
                                                       const MPI_Aint sdispls[], const MPI_Datatype sendtypes[],
                                                       void *recvbuf, const int recvcounts[],
                                                       const MPI_Aint rdispls[], const MPI_Datatype recvtypes[],
                                                       MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_INEIGHBOR_ALLTOALLW);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_INEIGHBOR_ALLTOALLW);

    mpi_errno = MPIR_Ineighbor_alltoallw_impl(sendbuf, sendcounts, sdispls, sendtypes,
                                         recvbuf, recvcounts, rdispls, recvtypes,
                                         comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_INEIGHBOR_ALLTOALLW);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ibarrier
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ibarrier(MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IBARRIER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IBARRIER);

    mpi_errno = MPIR_Ibarrier_impl(comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IBARRIER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ibcast
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ibcast(void *buffer, int count, MPI_Datatype datatype,
                                          int root, MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IBCAST);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IBCAST);

    mpi_errno = MPIR_Ibcast_impl(buffer, count, datatype, root, comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IBCAST);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_iallgather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_iallgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                              void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                              MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IALLGATHER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IALLGATHER);

    mpi_errno = MPIR_Iallgather_impl(sendbuf, sendcount, sendtype, recvbuf,
            recvcount, recvtype, comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IALLGATHER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_iallgatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_iallgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                               void *recvbuf, const int *recvcounts, const int *displs,
                                               MPI_Datatype recvtype, MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IALLGATHERV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IALLGATHERV);

    mpi_errno = MPIR_Iallgatherv_impl(sendbuf, sendcount, sendtype,
                                 recvbuf, recvcounts, displs, recvtype,
                                 comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IALLGATHERV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ialltoall
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ialltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                             void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                             MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IALLTOALL);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IALLTOALL);

    mpi_errno = MPIR_Ialltoall_impl(sendbuf, sendcount, sendtype, recvbuf,
                               recvcount, recvtype, comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IALLTOALL);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ialltoallv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ialltoallv(const void *sendbuf, const int *sendcounts,
                                              const int *sdispls, MPI_Datatype sendtype,
                                              void *recvbuf, const int *recvcounts,
                                              const int *rdispls, MPI_Datatype recvtype,
                                              MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IALLTOALLV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IALLTOALLV);

    mpi_errno = MPIR_Ialltoallv_impl(sendbuf, sendcounts, sdispls,
                                        sendtype, recvbuf, recvcounts,
                                        rdispls, recvtype, comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IALLTOALLV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ialltoallw
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ialltoallw(const void *sendbuf, const int *sendcounts,
                                              const int *sdispls, const MPI_Datatype sendtypes[],
                                              void *recvbuf, const int *recvcounts,
                                              const int *rdispls, const MPI_Datatype recvtypes[],
                                              MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_SHM_IALLTOALLW);
    MPIDI_FUNC_ENTER(MPID_STATE_SHM_IALLTOALLW);

    mpi_errno = MPIR_Ialltoallw_impl(sendbuf, sendcounts, sdispls,
                                        sendtypes, recvbuf, recvcounts,
                                        rdispls, recvtypes, comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_SHM_IALLTOALLW);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_iexscan
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_iexscan(const void *sendbuf, void *recvbuf, int count,
                                           MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr,
                                           MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IEXSCAN);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IEXSCAN);

    mpi_errno = MPIR_Iexscan_impl(sendbuf, recvbuf, count, datatype,
                             op, comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IEXSCAN);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_igather
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_igather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                           void *recvbuf, int recvcount, MPI_Datatype recvtype,
                                           int root, MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IGATHER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IGATHER);

    mpi_errno = MPIR_Igather_impl(sendbuf, sendcount, sendtype, recvbuf,
                             recvcount, recvtype, root, comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IGATHER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_igatherv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_igatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                            void *recvbuf, const int *recvcounts, const int *displs,
                                            MPI_Datatype recvtype, int root, MPIR_Comm *comm_ptr,
                                            MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IGATHERV);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IGATHERV);

    mpi_errno = MPIR_Igatherv_impl(sendbuf, sendcount, sendtype,
                              recvbuf, recvcounts, displs, recvtype,
                              root, comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IGATHERV);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ireduce_scatter_block
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ireduce_scatter_block(const void *sendbuf, void *recvbuf,
                                                         int recvcount, MPI_Datatype datatype,
                                                         MPI_Op op, MPIR_Comm *comm_ptr,
                                                         MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IREDUCE_SCATTER_BLOCK);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IREDUCE_SCATTER_BLOCK);

    mpi_errno = MPIR_Ireduce_scatter_block_impl(sendbuf, recvbuf, recvcount,
                                           datatype, op, comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IREDUCE_SCATTER_BLOCK);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ireduce_scatter
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ireduce_scatter(const void *sendbuf, void *recvbuf,
                                                   const int recvcounts[], MPI_Datatype datatype,
                                                   MPI_Op op, MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IREDUCE_SCATTER);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IREDUCE_SCATTER);

    mpi_errno = MPIR_Ireduce_scatter_impl(sendbuf, recvbuf, recvcounts, datatype, op,
                                     comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IREDUCE_SCATTER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_ireduce
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_ireduce(const void *sendbuf, void *recvbuf, int count,
                                           MPI_Datatype datatype, MPI_Op op, int root,
                                           MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IREDUCE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IREDUCE);

    mpi_errno = MPIR_Ireduce_impl(sendbuf, recvbuf, count, datatype,
                             op, root, comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IREDUCE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_iallreduce
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_iallreduce(const void *sendbuf, void *recvbuf, int count,
                                           MPI_Datatype datatype, MPI_Op op,
                                           MPIR_Comm *comm_ptr, MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_IALLREDUCE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_IALLREDUCE);

    mpi_errno = MPIR_Iallreduce_impl(sendbuf, recvbuf, count, datatype,
                                     op, comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_IALLREDUCE);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_iscan
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_iscan(const void *sendbuf, void *recvbuf, int count,
                                         MPI_Datatype datatype, MPI_Op op, MPIR_Comm *comm_ptr,
                                         MPI_Request * req)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_SHM_ISCAN);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_SHM_ISCAN);

    mpi_errno = MPIR_Iscan_impl(sendbuf, recvbuf, count, datatype,
                           op, comm_ptr, req);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_SHM_ISCAN);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_iscatter
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_iscatter(const void * sendbuf, int sendcount,
                                        MPI_Datatype sendtype, void * recvbuf,
                                        int recvcount, MPI_Datatype recvtype,
                                        int root, MPIR_Comm *comm, MPI_Request * request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_SHM_ISCATTER);
    MPIDI_FUNC_ENTER(MPID_STATE_SHM_ISCATTER);

    mpi_errno = MPIR_Iscatter_impl(sendbuf, sendcount, sendtype, recvbuf,
            recvcount, recvtype, root, comm, request);

    MPIDI_FUNC_EXIT(MPID_STATE_SHM_ISCATTER);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_SHM_iscatterv
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_SHM_iscatterv(const void * sendbuf, const int * sendcounts,
                                         const int * displs, MPI_Datatype sendtype,
                                         void * recvbuf, int recvcount,
                                         MPI_Datatype recvtype, int root,
                                         MPIR_Comm *comm, MPI_Request * request)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_SHM_ISCATTERV);
    MPIDI_FUNC_ENTER(MPID_STATE_SHM_ISCATTERV);

    mpi_errno = MPIR_Iscatterv_impl(sendbuf, sendcounts, displs, sendtype,
            recvbuf, recvcount, recvtype, root, comm, request);

    MPIDI_FUNC_EXIT(MPID_STATE_SHM_ISCATTERV);
    return mpi_errno;
}

#endif
