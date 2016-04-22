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
#ifndef CH4R_SYMHEAP_H_INCLUDED
#define CH4R_SYMHEAP_H_INCLUDED

#include <mpichconf.h>

#include <opa_primitives.h>
#ifdef HAVE_SYS_MMAN_H
#  include <sys/mman.h>
#endif /* HAVE_SYS_MMAN_H */
#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif /* HAVE_STDINT_H */

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_get_mapsize
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline size_t MPIDI_CH4R_get_mapsize(size_t size,
					    size_t *psz)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4R_GET_MAPSIZE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_GET_MAPSIZE);

    long    page_sz = sysconf(_SC_PAGESIZE);
    size_t  mapsize = (size + (page_sz-1))&(~(page_sz-1));
    *psz            = page_sz;

    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_GET_MAPSIZE);
    return mapsize;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_check_maprange_ok
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4R_check_maprange_ok(void   *start,
					       size_t  size)
{
    int     rc          = 0;
    int     ret         = 0;
    size_t  page_sz;
    size_t  mapsize     = MPIDI_CH4R_get_mapsize(size,&page_sz);
    size_t  i,num_pages = mapsize/page_sz;
    char   *ptr         = (char *)start;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_CHECK_MAPRANGE_OK);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_CHECK_MAPRANGE_OK);

    for(i=0; i<num_pages; i++) {
        rc = msync(ptr,page_sz,0);

        if(rc == -1) {
            MPIU_Assert(errno == ENOMEM);
            ptr+=page_sz;
        } else
            goto fn_exit;
    }

    ret = 1;
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_CHECK_MAPRANGE_OK);
    return ret;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_generate_random_addr
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline void *MPIDI_CH4R_generate_random_addr(size_t size)
{
    /* starting position for pointer to map
     * This is not generic, probably only works properly on Linux
     * but it's not fatal since we bail after a fixed number of iterations
     */
#define MPIDI_CH4I_MAP_POINTER ((random_unsigned&((0x00006FFFFFFFFFFF&(~(page_sz-1)))|0x0000600000000000)))
    char            random_state[256];
    size_t          page_sz;
    uint64_t        random_unsigned;
    size_t          mapsize     = MPIDI_CH4R_get_mapsize(size, &page_sz);
    uintptr_t       map_pointer;
    struct timeval  ts;
    int             iter = 100;
    int32_t         rh, rl;
    struct random_data rbuf;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_GENERATE_RANDOM_ADDR);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_GENERATE_RANDOM_ADDR);

    /* rbuf must be zero-cleared otherwise it results in SIGSEGV in glibc
       (http://stackoverflow.com/questions/4167034/c-initstate-r-crashing) */
    memset(&rbuf, 0, sizeof(rbuf));

    gettimeofday(&ts, NULL);

    initstate_r(ts.tv_usec,random_state,sizeof(random_state),&rbuf);
    random_r(&rbuf, &rh);
    random_r(&rbuf, &rl);
    random_unsigned  = ((uint64_t)rh)<<32|(uint64_t)rl;
    map_pointer = MPIDI_CH4I_MAP_POINTER;

    while(MPIDI_CH4R_check_maprange_ok((void *)map_pointer,mapsize) == 0) {
        random_r(&rbuf, &rh);
        random_r(&rbuf, &rl);
        random_unsigned  = ((uint64_t)rh)<<32|(uint64_t)rl;
        map_pointer = MPIDI_CH4I_MAP_POINTER;
        iter--;

        if(iter == 0) {
            map_pointer = -1ULL;
            goto fn_exit;
        }
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_GENERATE_RANDOM_ADDR);
    return (void *)map_pointer;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4R_get_symmetric_heap
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4R_get_symmetric_heap(MPI_Aint    size,
                                                MPIR_Comm  *comm,
                                                void      **base,
                                                MPIR_Win   *win)
{
    int mpi_errno = MPI_SUCCESS;
    unsigned  test, result;
    int       iter=100;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    void     *baseP;
    size_t    page_sz;
    size_t    mapsize;

    MPIDI_STATE_DECL(MPID_STATE_CH4R_GET_SYMMETRIC_HEAP);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4R_GET_SYMMETRIC_HEAP);

    mapsize = MPIDI_CH4R_get_mapsize(size, &page_sz);

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
                map_pointer = (uintptr_t)MPIDI_CH4R_generate_random_addr(mapsize);
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
                int rc = MPIDI_CH4R_check_maprange_ok((void *)map_pointer,mapsize);

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
        MPIDI_CH4U_WIN(win, mmap_sz)   = -1ULL;
        MPIDI_CH4U_WIN(win, mmap_addr) = NULL;
    } else {
        MPIDI_CH4U_WIN(win, mmap_sz)   = mapsize;
        MPIDI_CH4U_WIN(win, mmap_addr) = baseP;
    }

    *base = baseP;

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4R_GET_SYMMETRIC_HEAP);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#endif /* CH4R_SYMHEAP_H_INCLUDED */
