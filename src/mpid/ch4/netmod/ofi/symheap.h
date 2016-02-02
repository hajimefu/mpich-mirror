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
#ifndef NETMOD_OFI_SYMHEAP_H_INCLUDED
#define NETMOD_OFI_SYMHEAP_H_INCLUDED

#include "impl.h"
#include <opa_primitives.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

static inline size_t MPIDI_CH4_NMI_OFI_Get_mapsize(size_t size,
                                                   size_t *psz)
{
    long    page_sz = sysconf(_SC_PAGESIZE);
    size_t  mapsize = (size + (page_sz-1))&(~(page_sz-1));
    *psz            = page_sz;
    return mapsize;
}

static inline int MPIDI_CH4_NMI_OFI_Check_maprange(void   *start,
                                                   size_t  size)
{
    int     rc          = 0;
    size_t  page_sz;
    size_t  mapsize     = MPIDI_CH4_NMI_OFI_Get_mapsize(size,&page_sz);
    size_t  i,num_pages = mapsize/page_sz;
    char   *ptr         = (char *)start;

    for(i=0; i<num_pages; i++) {
        rc = msync(ptr,page_sz,0);

        if(rc == -1) {
            MPIU_Assert(errno == ENOMEM);
            ptr+=page_sz;
        } else
            return 0;
    }

    return 1;
}

static inline void *MPIDI_CH4_NMI_OFI_Generate_random_addr(size_t size)
{
    /* starting position for pointer to map
     * This is not generic, probably only works properly on Linux
     * but it's not fatal since we bail after a fixed number of iterations
     */
#define MPIDI_CH4_NMI_OFI_MAP_POINTER ((random_unsigned&((0x00006FFFFFFFFFFF&(~(page_sz-1)))|0x0000600000000000)))
    char            random_state[256];
    size_t          page_sz;
    uint64_t        random_unsigned;
    size_t          mapsize     = MPIDI_CH4_NMI_OFI_Get_mapsize(size,&page_sz);
    uintptr_t       map_pointer;
    struct timeval  ts;
    int             iter = 100;
    int32_t         rh, rl;
    struct random_data rbuf;

    /* rbuf must be zero-cleared otherwise it results in SIGSEGV in glibc
       (http://stackoverflow.com/questions/4167034/c-initstate-r-crashing) */
    memset(&rbuf, 0, sizeof(rbuf));

    gettimeofday(&ts, NULL);

    initstate_r(ts.tv_usec,random_state,sizeof(random_state),&rbuf);
    random_r(&rbuf, &rh);
    random_r(&rbuf, &rl);
    random_unsigned  = ((uint64_t)rh)<<32|(uint64_t)rl;
    map_pointer = MPIDI_CH4_NMI_OFI_MAP_POINTER;

    while(MPIDI_CH4_NMI_OFI_Check_maprange((void *)map_pointer,mapsize) == 0) {
        random_r(&rbuf, &rh);
        random_r(&rbuf, &rl);
        random_unsigned  = ((uint64_t)rh)<<32|(uint64_t)rl;
        map_pointer = MPIDI_CH4_NMI_OFI_MAP_POINTER;
        iter--;

        if(iter == 0)
            return (void *)-1ULL;
    }

    return (void *)map_pointer;
}

static inline int MPIDI_CH4_NMI_OFI_Get_symmetric_heap(MPI_Aint    size,
                                                       MPID_Comm  *comm,
                                                       void      **base,
                                                       MPID_Win   *win)
{
    int mpi_errno = MPI_SUCCESS;
    unsigned  test, result;
    int       iter=100;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    void     *baseP;
    size_t    page_sz;
    size_t    mapsize;

    mapsize = MPIDI_CH4_NMI_OFI_Get_mapsize(size, &page_sz);

    struct {
        uint64_t sz;
        int      loc;
    } maxloc, maxloc_result;

    maxloc.sz  = size;
    maxloc.loc = comm->rank;
    mpi_errno = MPIR_Allreduce_impl(&maxloc,
                                    &maxloc_result,
                                    1,
                                    MPI_LONG_INT,
                                    MPI_MAXLOC,
                                    comm,
                                    &errflag);

    if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

    if(maxloc_result.sz > 0) {
        result = 0;

        while(!result && --iter!=0) {
            uintptr_t  map_pointer = 0ULL;

            baseP                  = (void *)-1ULL;

            if(comm->rank == maxloc_result.loc) {
                map_pointer = (uintptr_t)MPIDI_CH4_NMI_OFI_Generate_random_addr(mapsize);
                baseP       = mmap((void *)map_pointer,
                                   mapsize,
                                   PROT_READ|PROT_WRITE,
                                   MAP_PRIVATE|MAP_ANON|MAP_FIXED,
                                   -1, 0);
            }

            mpi_errno = MPIR_Bcast_impl(&map_pointer,
                                        1,
                                        MPI_UNSIGNED_LONG,
                                        maxloc_result.loc,
                                        comm,&errflag);

            if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

            if(comm->rank != maxloc_result.loc) {
                int rc = MPIDI_CH4_NMI_OFI_Check_maprange((void *)map_pointer,mapsize);

                if(rc) {
                    baseP = mmap((void *)map_pointer,
                                 mapsize,
                                 PROT_READ|PROT_WRITE,
                                 MAP_PRIVATE|MAP_ANON|MAP_FIXED,
                                 -1, 0);
                } else
                    baseP = (void *)-1ULL;
            }

            if(mapsize == 0) baseP = (void *)map_pointer;

            test = ((uintptr_t)baseP != -1ULL)?1:0;
            mpi_errno = MPIR_Allreduce_impl(&test,
                                            &result,
                                            1,
                                            MPI_UNSIGNED,
                                            MPI_BAND,
                                            comm,
                                            &errflag);

            if(mpi_errno!=MPI_SUCCESS) goto fn_fail;

            if(result == 0 && baseP!=(void *)-1ULL)
                munmap(baseP, mapsize);
        }
    } else
        baseP = NULL;

    if(iter == 0) {
        fprintf(stderr, "WARNING: Win_allocate:  Unable to allocate symmetric heap\n");
        baseP = MPL_malloc(size);
        MPIR_ERR_CHKANDJUMP((baseP == NULL), mpi_errno,
                            MPI_ERR_BUFFER, "**bufnull");
        MPIDI_CH4R_WIN(win, mmap_sz)   = -1ULL;
        MPIDI_CH4R_WIN(win, mmap_addr) = NULL;
    } else {
        MPIDI_CH4R_WIN(win, mmap_sz)   = mapsize;
        MPIDI_CH4R_WIN(win, mmap_addr) = baseP;
    }

    *base = baseP;

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#endif
