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
#ifndef MPIDCH4_INIT_H_INCLUDED
#define MPIDCH4_INIT_H_INCLUDED

#include "ch4_impl.h"

/*
=== BEGIN_MPI_T_CVAR_INFO_BLOCK ===

cvars:
    - name        : MPIR_CVAR_NETMOD
      category    : NEMESIS
      type        : string
      default     : ""
      class       : device
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        If non-empty, this cvar specifies which network module to use
    - name        : MPIR_CVAR_SHM
      category    : NEMESIS
      type        : string
      default     : ""
      class       : device
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        If non-empty, this cvar specifies which shm module to use

=== END_MPI_T_CVAR_INFO_BLOCK ===
*/

#undef FUNCNAME
#define FUNCNAME MPIDI_choose_netmod
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_choose_netmod(void)
{
    int i, mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4_CHOOSE_NETMOD);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_CHOOSE_NETMOD);

    MPIU_Assert(MPIR_CVAR_NETMOD != NULL);

    if (strcmp(MPIR_CVAR_NETMOD, "") == 0) {
        /* netmod not specified, using the default */
        MPIDI_netmod_func = MPIDI_netmod_funcs[0];
        MPIDI_netmod_native_func = MPIDI_netmod_native_funcs[0];
        goto fn_exit;
    }

    for (i = 0; i < MPIDI_num_netmods; ++i) {
        if (!MPIU_Strncasecmp
            (MPIR_CVAR_NETMOD, MPIDI_netmod_strings[i], MPIDI_MAX_NETMOD_STRING_LEN)) {
            MPIDI_netmod_func = MPIDI_netmod_funcs[i];
            MPIDI_netmod_native_func = MPIDI_netmod_native_funcs[i];
            goto fn_exit;
        }
    }

    MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**invalid_netmod", "**invalid_netmod %s",
                         MPIR_CVAR_NETMOD);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_CHOOSE_NETMOD);
    return mpi_errno;
  fn_fail:

    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_choose_shm
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_choose_shm(void)
{

    int mpi_errno = MPI_SUCCESS;
#if defined(MPIDI_BUILD_CH4_SHM) || defined(MPIDI_CH4_EXCLUSIVE_SHM)
    int i;
    MPIDI_STATE_DECL(MPID_STATE_CH4_CHOOSE_SHM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_CHOOSE_SHM);


    MPIU_Assert(MPIR_CVAR_SHM != NULL);

    if (strcmp(MPIR_CVAR_SHM, "") == 0) {
        /* shm not specified, using the default */
        MPIDI_shm_func = MPIDI_shm_funcs[0];
        MPIDI_shm_native_func = MPIDI_shm_native_funcs[0];
        goto fn_exit;
    }

    for (i = 0; i < MPIDI_num_shms; ++i) {
        if (!MPIU_Strncasecmp
            (MPIR_CVAR_SHM, MPIDI_shm_strings[i], MPIDI_MAX_SHM_STRING_LEN)) {
            MPIDI_shm_func = MPIDI_shm_funcs[i];
            MPIDI_shm_native_func = MPIDI_shm_native_funcs[i];
            goto fn_exit;
        }
    }

    MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**ch4|invalid_shm", "**ch4|invalid_shm %s",
                         MPIR_CVAR_SHM);
  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_CHOOSE_SHM);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
#else
    return mpi_errno;
#endif
}


#if (MPIU_THREAD_GRANULARITY == MPIU_THREAD_GRANULARITY_PER_OBJECT)
#define MAX_THREAD_MODE MPI_THREAD_MULTIPLE
#elif  (MPIU_THREAD_GRANULARITY == MPIU_THREAD_GRANULARITY_GLOBAL)
#define MAX_THREAD_MODE MPI_THREAD_MULTIPLE
#elif  (MPIU_THREAD_GRANULARITY == MPIU_THREAD_GRANULARITY_SINGLE)
#define MAX_THREAD_MODE MPI_THREAD_SERIALIZED
#elif  (MPIU_THREAD_GRANULARITY == MPIU_THREAD_GRANULARITY_LOCK_FREE)
#define MAX_THREAD_MODE MPI_THREAD_SERIALIZED
#else
#error "Thread Granularity:  Invalid"
#endif

#undef FUNCNAME
#define FUNCNAME MPIDI_Init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Init(int *argc,
                              char ***argv,
                              int requested, int *provided, int *has_args, int *has_env)
{
    int pmi_errno, mpi_errno = MPI_SUCCESS, rank, has_parent, size, appnum, i;
    void *netmod_contexts;

    MPIDI_STATE_DECL(MPID_STATE_CH4_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_INIT);

    MPIDI_choose_netmod();
    pmi_errno = PMI_Init(&has_parent);

    if (pmi_errno != PMI_SUCCESS) {
        MPIR_ERR_SETANDJUMP1(pmi_errno, MPI_ERR_OTHER, "**pmi_init", "**pmi_init %d", pmi_errno);
    }

    pmi_errno = PMI_Get_rank(&rank);

    if (pmi_errno != PMI_SUCCESS) {
        MPIR_ERR_SETANDJUMP1(pmi_errno, MPI_ERR_OTHER, "**pmi_get_rank",
                             "**pmi_get_rank %d", pmi_errno);
    }

    pmi_errno = PMI_Get_size(&size);

    if (pmi_errno != 0) {
        MPIR_ERR_SETANDJUMP1(pmi_errno, MPI_ERR_OTHER, "**pmi_get_size",
                             "**pmi_get_size %d", pmi_errno);
    }

    pmi_errno = PMI_Get_appnum(&appnum);

    if (pmi_errno != PMI_SUCCESS) {
        MPIR_ERR_SETANDJUMP1(pmi_errno, MPI_ERR_OTHER, "**pmi_get_appnum",
                             "**pmi_get_appnum %d", pmi_errno);
    }

    MPIDI_CH4_Global.comms = (MPID_Comm **) MPIU_Calloc(1, sizeof(MPID_Comm *)
                                                        * MPIR_CONTEXT_INT_BITS *
                                                        MPIR_CONTEXT_ID_BITS);
#if defined(MPIDI_BUILD_CH4_LOCALITY_INFO)
    /* Create and initialize the locality array */
    /* TODO: This doesn't support dynamic processes */
    MPIDI_CH4U_gpid_local = (int *) MPIU_Malloc(sizeof(int) * size);
    for (i = 0; i < size; i++)
        MPIDI_CH4U_gpid_local[i] = FALSE;
#endif

#if defined(MPIDI_BUILD_CH4_SHM) || defined(MPIDI_CH4_EXCLUSIVE_SHM)
    /* shm mechanism has to be choosen before netmod initialization phase because of
       shm functions dynamic binding and a fact that netmod initialization uses
       a collective operation whcih may involve shm related functions like progress engine */
    MPIDI_choose_shm();
#endif

    mpi_errno = MPIDI_netmod_init(rank, size, appnum, &MPIR_Process.attrs.tag_ub,
                                  MPIR_Process.comm_world,
                                  MPIR_Process.comm_self, 1, &netmod_contexts);

#if defined(MPIDI_BUILD_CH4_SHM) || defined(MPIDI_CH4_EXCLUSIVE_SHM)
    mpi_errno = MPIDI_shm_init(rank, size);
#endif
#if 0
#if defined(MPIDI_BUILD_CH4_LOCALITY_INFO)
    /* Populate the locality array */
    for (i = 0; i < size; i++)
        MPIDI_CH4U_gpid_local[i] = FALSE;
#endif
#endif

    MPIR_Process.attrs.tag_ub = (1 << MPIDI_CH4U_TAG_SHIFT) - 1;
    /* discuss */

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

    MPIR_Process.attrs.appnum = appnum;
    MPIR_Process.attrs.wtime_is_global = 1;
    MPIR_Process.attrs.io = MPI_ANY_SOURCE;

    /* -------------------------------- */
    /* Return MPICH Parameters          */
    /* -------------------------------- */
    switch (requested) {
    case MPI_THREAD_SINGLE:
    case MPI_THREAD_SERIALIZED:
    case MPI_THREAD_FUNNELED:
        *provided = requested;
        break;

    case MPI_THREAD_MULTIPLE:
        *provided = MAX_THREAD_MODE;
        break;
    }

    *has_args = TRUE;
    *has_env = TRUE;
    MPIDI_CH4_Global.is_initialized = 0;

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_INIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_InitCompleted
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_InitCompleted(void)
{
    MPIDI_STATE_DECL(MPID_STATE_CH4_INITCOMPLETED);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_INITCOMPLETED);
    MPIDI_CH4_Global.is_initialized = 1;
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_INITCOMPLETED);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Finalize
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Finalize(void)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_FINALIZE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_FINALIZE);
    mpi_errno = MPIDI_netmod_finalize();

#if defined(MPIDI_BUILD_CH4_SHM) || defined(MPIDI_CH4_EXCLUSIVE_SHM)
    mpi_errno = MPIDI_shm_finalize();
#endif

    MPIU_Free(MPIDI_CH4_Global.comms);
#if defined(MPIDI_BUILD_CH4_LOCALITY_INFO)
    MPIU_Free(MPIDI_CH4U_gpid_local);
#endif

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_FINALIZE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Get_universe_size
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Get_universe_size(int *universe_size)
{
    int mpi_errno = MPI_SUCCESS;
    int pmi_errno = PMI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4_GET_UNIVERSE_SIZE);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_GET_UNIVERSE_SIZE);


    pmi_errno = PMI_Get_universe_size(universe_size);

    if (pmi_errno != PMI_SUCCESS)
        MPIR_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER,
                             "**pmi_get_universe_size", "**pmi_get_universe_size %d", pmi_errno);

    if (*universe_size < 0)
        *universe_size = MPIR_UNIVERSE_SIZE_NOT_AVAILABLE;

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_GET_UNIVERSE_SIZE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
    int err = MPI_SUCCESS;
    return err;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Get_processor_name
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Get_processor_name(char *name, int namelen, int *resultlen)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_CH4_GET_PROCESSOR_NAME);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_GET_PROCESSOR_NAME);

    if (!MPIDI_CH4_Global.pname_set) {
#if defined(HAVE_GETHOSTNAME)

        if (gethostname(MPIDI_CH4_Global.pname, MPI_MAX_PROCESSOR_NAME) == 0)
            MPIDI_CH4_Global.pname_len = (int) strlen(MPIDI_CH4_Global.pname);

#elif defined(HAVE_SYSINFO)

        if (sysinfo(SI_HOSTNAME, MPIDI_CH4_Global.pname, MPI_MAX_PROCESSOR_NAME) == 0)
            MPIDI_CH4_Global.pname_len = (int) strlen(MPIDI_CH4_Global.pname);

#else
        MPL_snprintf(MPIDI_CH4_Global.pname, MPI_MAX_PROCESSOR_NAME, "%d",
                     MPIR_Process.comm_world->rank);
        MPIDI_CH4_Global.pname_len = (int) strlen(MPIDI_CH4_Global.pname);
#endif
        MPIDI_CH4_Global.pname_set = 1;
    }

    MPIR_ERR_CHKANDJUMP(MPIDI_CH4_Global.pname_len <= 0,
                        mpi_errno, MPI_ERR_OTHER, "**procnamefailed");
    MPIU_Strncpy(name, MPIDI_CH4_Global.pname, namelen);

    if (resultlen)
        *resultlen = MPIDI_CH4_Global.pname_len;

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_GET_PROCESSOR_NAME);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Abort
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Abort(MPID_Comm * comm,
                               int mpi_errno, int exit_code, const char *error_msg)
{
    char sys_str[MPI_MAX_ERROR_STRING + 5] = "";
    char comm_str[MPI_MAX_ERROR_STRING] = "";
    char world_str[MPI_MAX_ERROR_STRING] = "";
    char error_str[2 * MPI_MAX_ERROR_STRING + 128];
    MPIDI_STATE_DECL(MPID_STATE_CH4_ABORT);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_ABORT);

    if (MPIR_Process.comm_world) {
        int rank = MPIR_Process.comm_world->rank;
        snprintf(world_str, sizeof(world_str), " on node %d", rank);
    }

    if (comm) {
        int rank = comm->rank;
        int context_id = comm->context_id;
        snprintf(comm_str, sizeof(comm_str), " (rank %d in comm %d)", rank, context_id);
    }

    if (!error_msg)
        error_msg = "Internal error";

    if (mpi_errno != MPI_SUCCESS) {
        char msg[MPI_MAX_ERROR_STRING] = "";
        MPIR_Err_get_string(mpi_errno, msg, MPI_MAX_ERROR_STRING, NULL);
        snprintf(sys_str, sizeof(msg), " (%s)", msg);
    }

    snprintf(error_str, sizeof(error_str), "Abort(%d)%s%s: %s%s\n",
             exit_code, world_str, comm_str, error_msg, sys_str);
    MPL_error_printf("%s", error_str);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_ABORT);
    fflush(stderr);
    fflush(stdout);
    exit(1);
    return 0;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Alloc_mem
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ void *MPIDI_Alloc_mem(size_t size, MPID_Info * info_ptr)
{
    void *p;
    MPIDI_STATE_DECL(MPID_STATE_CH4_ALLOC_MEM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_ALLOC_MEM);

    p = MPIDI_netmod_alloc_mem(size, info_ptr);

    MPIDI_FUNC_EXIT(MPID_STATE_CH4_ALLOC_MEM);
    return p;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Free_mem
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Free_mem(void *ptr)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_FREE_MEM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_FREE_MEM);
    mpi_errno = MPIDI_netmod_free_mem(ptr);

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_FREE_MEM);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Comm_get_lpid
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Comm_get_lpid(MPID_Comm * comm_ptr,
                                       int idx, int *lpid_ptr, MPIU_BOOL is_remote)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_COMM_GET_LPID);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_COMM_GET_LPID);
    mpi_errno = MPIDI_netmod_comm_get_lpid(comm_ptr, idx, lpid_ptr, is_remote);

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_COMM_GET_LPID);
    return mpi_errno;
  fn_fail:
    goto fn_exit;

}

#undef FUNCNAME
#define FUNCNAME MPIDI_GPID_Get
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_GPID_Get(MPID_Comm * comm_ptr, int rank, MPID_Gpid * gpid)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_GPID_GET);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_GPID_GET);
    mpi_errno = MPIDI_netmod_gpid_get(comm_ptr, rank, gpid);

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_GPID_GET);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Get_node_id
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Get_node_id(MPID_Comm * comm, int rank, MPID_Node_id_t * id_p)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_GET_NODE_ID);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_GET_NODE_ID);
    mpi_errno = MPIDI_netmod_get_node_id(comm, rank, id_p);

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_GET_NODE_ID);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Get_max_node_id
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Get_max_node_id(MPID_Comm * comm, MPID_Node_id_t * max_id_p)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_GET_MAX_NODE_ID);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_GET_MAX_NODE_ID);
    mpi_errno = MPIDI_netmod_get_max_node_id(comm, max_id_p);

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_GET_MAX_NODE_ID);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_GetAllInComm
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_GPID_GetAllInComm(MPID_Comm * comm_ptr,
                                           int local_size, MPID_Gpid local_gpids[], int *singlePG)
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_GETALLINCOMM);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_GETALLINCOMM);
    mpi_errno = MPIDI_netmod_getallincomm(comm_ptr, local_size, local_gpids, singlePG);

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_GETALLINCOMM);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_GPID_ToLpidArray
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_GPID_ToLpidArray(int size, MPID_Gpid gpid[], int lpid[])
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_GPID_TOLPIDARRAY);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_GPID_TOLPIDARRAY);
    mpi_errno = MPIDI_netmod_gpid_tolpidarray(size, gpid, lpid);

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_GPID_TOLPIDARRAY);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Create_intercomm_from_lpids
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ int MPIDI_Create_intercomm_from_lpids(MPID_Comm * newcomm_ptr,
                                                     int size, const int lpids[])
{
    int mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_CH4_CREATE_INTERCOMM_FROM_LPIDS);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_CREATE_INTERCOMM_FROM_LPIDS);
    mpi_errno = MPIDI_netmod_create_intercomm_from_lpids(newcomm_ptr, size, lpids);

    if (mpi_errno != MPI_SUCCESS) {
        MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_CREATE_INTERCOMM_FROM_LPIDS);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_Aint_add
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ MPI_Aint MPIDI_Aint_add(MPI_Aint base, MPI_Aint disp)
{
    MPI_Aint result;
    MPIDI_STATE_DECL(MPID_STATE_CH4_AINT_ADD);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_AINT_ADD);
    result = MPIU_VOID_PTR_CAST_TO_MPI_AINT((char *) MPIU_AINT_CAST_TO_VOID_PTR(base) + disp);
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_AINT_ADD);
    return result;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Aint_diff
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
__CH4_INLINE__ MPI_Aint MPIDI_Aint_diff(MPI_Aint addr1, MPI_Aint addr2)
{
    MPI_Aint result;
    MPIDI_STATE_DECL(MPID_STATE_CH4_AINT_DIFF);
    MPIDI_FUNC_ENTER(MPID_STATE_CH4_AINT_DIFF);

    result = MPIU_PTR_DISP_CAST_TO_MPI_AINT((char *) MPIU_AINT_CAST_TO_VOID_PTR(addr1)
                                           - (char *) MPIU_AINT_CAST_TO_VOID_PTR(addr2));
    MPIDI_FUNC_EXIT(MPID_STATE_CH4_AINT_DIFF);
    return result;
}

#endif /* MPIDCH4_INIT_H_INCLUDED */
