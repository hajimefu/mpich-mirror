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
#ifndef MPIDCH4_SPAWN_H_INCLUDED
#define MPIDCH4_SPAWN_H_INCLUDED

#include "ch4_impl.h"

extern int  MPIR_Info_get_impl(MPID_Info *info_ptr, const char *key, int valuelen, char *value, int *flag);
extern void MPIR_Info_get_valuelen_impl(MPID_Info *info_ptr, const char *key, int *valuelen, int *flag);
extern void MPIR_Info_get_nkeys_impl(MPID_Info *info_ptr, int *nkeys);
extern int  MPIR_Info_get_nthkey_impl(MPID_Info *info, int n, char *key);
extern int  MPIR_Bcast_impl(void *buffer, int count, MPI_Datatype datatype, int root,
                            MPID_Comm *comm_ptr, MPIR_Errflag_t *errflag);
extern int MPIR_Comm_create(MPID_Comm **);

static inline int MPIDI_mpi_to_pmi_keyvals(MPID_Info     *info_ptr,
                                           PMI_keyval_t **kv_ptr,
                                           int           *nkeys_ptr)
{
    char key[MPI_MAX_INFO_KEY];
    PMI_keyval_t *kv = 0;
    int           i, nkeys = 0, vallen, flag, mpi_errno=MPI_SUCCESS;

    if(!info_ptr || info_ptr->handle == MPI_INFO_NULL)
        goto fn_exit;

    MPIR_Info_get_nkeys_impl(info_ptr, &nkeys);

    if(nkeys == 0) goto fn_exit;

    kv = (PMI_keyval_t *)MPIU_Malloc(nkeys * sizeof(PMI_keyval_t));

    for(i=0; i<nkeys; i++) {
        MPIDU_RC_POP(MPIR_Info_get_nthkey_impl(info_ptr,i,key));
        MPIR_Info_get_valuelen_impl(info_ptr,key,&vallen,&flag);
        kv[i].key = (const char *)MPIU_Strdup(key);
        kv[i].val = (char *)MPIU_Malloc(vallen + 1);
        MPIR_Info_get_impl(info_ptr, key, vallen+1, kv[i].val, &flag);
    }

fn_fail:
fn_exit:
    *kv_ptr    = kv;
    *nkeys_ptr = nkeys;
    return mpi_errno;
}

static inline void MPIDI_free_pmi_keyvals(PMI_keyval_t **kv,
                                          int            size,
                                          int           *counts)
{
    int i,j;

    for(i=0; i<size; i++) {
        for(j=0; j<counts[i]; j++) {
            if(kv[i][j].key != NULL)
                MPIU_Free((char *)kv[i][j].key);

            if(kv[i][j].val != NULL)
                MPIU_Free(kv[i][j].val);
        }

        if(kv[i] != NULL)
            MPIU_Free(kv[i]);
    }

}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_spawn_multiple
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_spawn_multiple(int         count,
                                             char       *commands[],
                                             char      **argvs[],
                                             const int   maxprocs[],
                                             MPID_Info  *info_ptrs[],
                                             int         root,
                                             MPID_Comm  *comm_ptr,
                                             MPID_Comm **intercomm,
                                             int         errcodes[])
{
    char           port_name[MPI_MAX_PORT_NAME];
    int           *info_keyval_sizes=0, i, mpi_errno=MPI_SUCCESS;
    PMI_keyval_t **info_keyval_vectors=0, preput_keyval_vector;
    int           *pmi_errcodes = 0, pmi_errno=0;
    int            total_num_processes, should_accept = 1;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_SPAWN_MULTIPLE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_SPAWN_MULTIPLE);

    memset(port_name, 0, sizeof(port_name));

    if(comm_ptr->rank == root) {
        total_num_processes = 0;

        for(i=0; i<count; i++)
            total_num_processes += maxprocs[i];

        pmi_errcodes = (int *)MPIU_Malloc(sizeof(int) * total_num_processes);
        MPIR_ERR_CHKANDJUMP(!pmi_errcodes, mpi_errno, MPI_ERR_OTHER, "**nomem");

        for(i=0; i<total_num_processes; i++)
            pmi_errcodes[i] = 0;

        MPIDU_RC_POP(MPIDI_Open_port(NULL, port_name));

        info_keyval_sizes   = (int *)          MPIU_Malloc(count*sizeof(int));
        MPIR_ERR_CHKANDJUMP(!info_keyval_sizes, mpi_errno, MPI_ERR_OTHER, "**nomem");
        info_keyval_vectors = (PMI_keyval_t **) MPIU_Malloc(count*sizeof(PMI_keyval_t *));
        MPIR_ERR_CHKANDJUMP(!info_keyval_vectors, mpi_errno, MPI_ERR_OTHER, "**nomem");

        if(!info_ptrs)
            for(i=0; i<count; i++) {
                info_keyval_vectors[i] = 0;
                info_keyval_sizes[i]   = 0;
            }
        else
            for(i=0; i<count; i++)
                MPIDU_RC_POP(MPIDI_mpi_to_pmi_keyvals(info_ptrs[i],
                                                          &info_keyval_vectors[i],
                                                          &info_keyval_sizes[i]));

        preput_keyval_vector.key = MPIDI_PARENT_PORT_KVSKEY;
        preput_keyval_vector.val = port_name;
        pmi_errno                = PMI_Spawn_multiple(count, (const char **)
                                                      commands,
                                                      (const char ** *) argvs,
                                                      maxprocs,
                                                      info_keyval_sizes,
                                                      (const PMI_keyval_t **)
                                                      info_keyval_vectors,
                                                      1,
                                                      &preput_keyval_vector,
                                                      pmi_errcodes);

        if(pmi_errno != PMI_SUCCESS)
            MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                                 "**pmi_spawn_multiple",
                                 "**pmi_spawn_multiple %d",
                                 pmi_errno);

        if(errcodes != MPI_ERRCODES_IGNORE) {
            for(i=0; i<total_num_processes; i++) {
                errcodes[i] = pmi_errcodes[0];
                should_accept = should_accept && errcodes[i];
            }

            should_accept = !should_accept;
        }
    }

    if(errcodes != MPI_ERRCODES_IGNORE) {
        MPIR_Errflag_t errflag = MPIR_ERR_NONE;
        MPIDU_RC_POP(MPIR_Bcast_impl(&should_accept,1,MPI_INT,
                                         root,comm_ptr,&errflag));
        MPIDU_RC_POP(MPIR_Bcast_impl(&pmi_errno,1,MPI_INT,
                                         root,comm_ptr,&errflag));
        MPIDU_RC_POP(MPIR_Bcast_impl(&total_num_processes,1,MPI_INT,
                                         root, comm_ptr, &errflag));
        MPIDU_RC_POP(MPIR_Bcast_impl(errcodes, total_num_processes, MPI_INT,
                                         root, comm_ptr,&errflag));
    }

    if(should_accept)
        MPIDU_RC_POP(MPIDI_Comm_accept(port_name, NULL, root,
                                           comm_ptr, intercomm));
    else {
        if((pmi_errno == PMI_SUCCESS) && (errcodes[0] != 0))
            MPIR_Comm_create(intercomm);
    }

    if(comm_ptr->rank == root)
        MPIDU_RC_POP(MPIDI_Close_port(port_name));

fn_exit:

    if(info_keyval_vectors) {
        MPIDI_free_pmi_keyvals(info_keyval_vectors, count, info_keyval_sizes);
        MPIU_Free(info_keyval_vectors);
    }

    if(info_keyval_sizes)
        MPIU_Free(info_keyval_sizes);

    if(pmi_errcodes)
        MPIU_Free(pmi_errcodes);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_SPAWN_MULTIPLE);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_connect
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_connect(const char *port_name,
                                      MPID_Info *info,
                                      int root, MPID_Comm *comm, MPID_Comm **newcomm_ptr)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_CONNECT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_CONNECT);
    mpi_errno = MPIDI_netmod_comm_connect(port_name, info, root, comm, newcomm_ptr);

    if(mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_CONNECT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_disconnect
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_disconnect(MPID_Comm *comm_ptr)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_DISCONNECT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_DISCONNECT);
    mpi_errno = MPIDI_netmod_comm_disconnect(comm_ptr);

    if(mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_DISCONNECT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Open_port
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Open_port(MPID_Info *info_ptr, char *port_name)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_OPEN_PORT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_OPEN_PORT);
    mpi_errno = MPIDI_netmod_open_port(info_ptr, port_name);

    if(mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_OPEN_PORT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Close_port
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Close_port(const char *port_name)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_CLOSE_PORT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_CLOSE_PORT);
    mpi_errno = MPIDI_netmod_close_port(port_name);

    if(mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_CLOSE_PORT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_accept
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_accept(const char *port_name,
                                     MPID_Info *info,
                                     int root, MPID_Comm *comm, MPID_Comm **newcomm_ptr)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_ACCEPT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_ACCEPT);
    mpi_errno = MPIDI_netmod_comm_accept(port_name, info, root, comm, newcomm_ptr);

    if(mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_ACCEPT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#endif /* MPIDCH4_SPAWN_H_INCLUDED */
