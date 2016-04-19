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

/* All global ADI data structures need to go in this file */
/* reference them with externs from other files           */

#include <mpidimpl.h>
#include "ch4_impl.h"

MPIDI_CH4_Global_t MPIDI_CH4_Global;

MPIDI_CH4_NM_funcs_t *MPIDI_CH4_NM_func;
MPIDI_CH4_NM_native_funcs_t *MPIDI_CH4_NM_native_func;

#ifdef MPIDI_BUILD_CH4_SHM
MPIDI_CH4_SHM_funcs_t        *MPIDI_CH4_SHM_func;
MPIDI_CH4_SHM_native_funcs_t *MPIDI_CH4_SHM_native_func;
#endif

#ifdef MPID_DEVICE_DEFINES_THREAD_CS
pthread_mutex_t MPIDI_Mutex_lock[MPIDI_NUM_LOCKS];
#endif

/* The MPID_Abort ADI is strangely defined by the upper layers */
/* We should fix the upper layer to define MPID_Abort like any */
/* Other ADI */
#ifdef MPID_Abort
#define MPID_TMP MPID_Abort
#undef MPID_Abort
int MPID_Abort(MPIR_Comm * comm, int mpi_errno, int exit_code, const char *error_msg)
{
    return MPIDI_Abort(comm, mpi_errno, exit_code, error_msg);
}

#define MPID_Abort MPID_TMP
#endif

/* Another weird ADI that doesn't follow convention */
static void init_comm()__attribute__((constructor));
static void init_comm()
{
  MPIR_Comm_fns             = &MPIDI_CH4_Global.MPIR_Comm_fns_store;
  MPIR_Comm_fns->split_type =  MPIDI_Comm_split_type;
}

MPL_dbg_class MPIDI_CH4_DBG_GENERAL;

void MPID_Request_init(MPIR_Request *req)
{
    MPIDI_CH4U_REQUEST(req, req) = NULL;
#ifdef MPIDI_BUILD_CH4_SHM
    MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req) = NULL;
#endif

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPID_Request_mem = MPIDI_Request_mem
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF MPID_Request_mem  MPIDI_Request_mem
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPID_Request_mem as MPIDI_Request_mem
#elif defined(HAVE_WEAK_ATTRIBUTE)
extern MPIU_Object_alloc_t MPID_Request_mem __attribute__((weak,alias("MPIDI_Request_mem")));
#endif
#ifndef MPICH_MPI_FROM_PMPI
#undef MPID_Request_mem
#define MPID_Request_mem MPIDI_Request_mem
#endif

#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPID_Request_direct = MPIDI_Request_direct
#elif defined(HAVE_PRAGMA_HP_SEC_DEF)
#pragma _HP_SECONDARY_DEF MPID_Request_direct  MPIDI_Request_direct
#elif defined(HAVE_PRAGMA_CRI_DUP)
#pragma _CRI duplicate MPID_Request_direct as MPIDI_Request_direct
#elif defined(HAVE_WEAK_ATTRIBUTE)
extern MPID_Request        MPID_Request_direct[MPID_REQUEST_PREALLOC] __attribute__((weak,alias("MPIDI_Request_direct")));
#endif
#ifndef MPICH_MPI_FROM_PMPI
#undef MPID_Request_direct
#define MPID_Request_direct MPIDI_Request_direct
#endif


void MPID_Request_finalize(MPIR_Request *req)
{
    return;
}
