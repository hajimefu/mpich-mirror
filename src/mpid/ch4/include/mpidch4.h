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

#if !defined(MPICH_MPIDCH4_H_INCLUDED)
#define MPICH_MPIDCH4_H_INCLUDED

#define __CH4_INLINE__ __attribute__((always_inline)) static inline

/* We need to define the static inlines right away to avoid
 * any implicit prototype generation and subsequent warnings
 * This allows us to make ADI up calls from within a direct
 * netmod.
 */
#define MPIDI_CH4_API(rc,fcnname,...)            \
  __CH4_INLINE__ rc MPIDI_##fcnname(__VA_ARGS__) \
  __attribute__((always_inline))

MPIDI_CH4_API(int, Init, int *, char ***, int, int *, int *, int *);
MPIDI_CH4_API(int, InitCompleted, void);
MPIDI_CH4_API(int, Abort, MPID_Comm *, int, int, const char *);
MPIDI_CH4_API(int, Cancel_recv, MPID_Request *);
MPIDI_CH4_API(int, Cancel_send, MPID_Request *);
MPIDI_CH4_API(int, Comm_disconnect, MPID_Comm *);
MPIDI_CH4_API(int, Comm_spawn_multiple, int, char *[], char **[], const int[], MPID_Info *[], int,
              MPID_Comm *, MPID_Comm **, int[]);
MPIDI_CH4_API(int, Comm_failure_get_acked, MPID_Comm *, MPID_Group **);
MPIDI_CH4_API(int, Comm_get_all_failed_procs, MPID_Comm *, MPID_Group **, int);
MPIDI_CH4_API(int, Comm_revoke, MPID_Comm *, int);
MPIDI_CH4_API(int, Comm_failure_ack, MPID_Comm *);
MPIDI_CH4_API(int, Comm_AS_enabled, MPID_Comm *);
MPIDI_CH4_API(int, Comm_get_lpid, MPID_Comm *, int, int *, MPIU_BOOL);
MPIDI_CH4_API(int, Finalize, void);
MPIDI_CH4_API(int, Get_universe_size, int *);
MPIDI_CH4_API(int, Get_processor_name, char *, int, int *);
MPIDI_CH4_API(int, Iprobe, int, int, MPID_Comm *, int, int *, MPI_Status *);
MPIDI_CH4_API(int, Irecv, void *, int, MPI_Datatype, int, int, MPID_Comm *, int, MPID_Request **);
MPIDI_CH4_API(int, Isend, const void *, int, MPI_Datatype, int, int, MPID_Comm *, int,
              MPID_Request **);
MPIDI_CH4_API(int, Issend, const void *, int, MPI_Datatype, int, int, MPID_Comm *, int,
              MPID_Request **);
MPIDI_CH4_API(int, Mrecv, void *, int, MPI_Datatype, MPID_Request *, MPI_Status *);
MPIDI_CH4_API(int, Imrecv, void *, int, MPI_Datatype, MPID_Request *, MPID_Request **);
MPIDI_CH4_API(int, Open_port, MPID_Info *, char *);
MPIDI_CH4_API(int, Close_port, const char *);
MPIDI_CH4_API(int, Comm_accept, const char *, MPID_Info *, int, MPID_Comm *, MPID_Comm **);
MPIDI_CH4_API(int, Comm_connect, const char *, MPID_Info *, int, MPID_Comm *, MPID_Comm **);
MPIDI_CH4_API(int, Probe, int, int, MPID_Comm *, int, MPI_Status *);
MPIDI_CH4_API(int, Mprobe, int, int, MPID_Comm *, int, MPID_Request **, MPI_Status *);
MPIDI_CH4_API(int, Improbe, int, int, MPID_Comm *, int, int *, MPID_Request **, MPI_Status *);
MPIDI_CH4_API(int, Progress_test, void);
MPIDI_CH4_API(int, Progress_poke, void);
MPIDI_CH4_API(void, Progress_start, MPID_Progress_state *);
MPIDI_CH4_API(void, Progress_end, MPID_Progress_state *);
MPIDI_CH4_API(int, Progress_wait, MPID_Progress_state *);
MPIDI_CH4_API(int, Progress_register, int (*progress_fn) (int *), int *id);
MPIDI_CH4_API(int, Progress_deregister, int id);
MPIDI_CH4_API(int, Progress_activate, int id);
MPIDI_CH4_API(int, Progress_deactivate, int id);
MPIDI_CH4_API(int, Recv, void *, int, MPI_Datatype, int, int, MPID_Comm *, int, MPI_Status *,
              MPID_Request **);
MPIDI_CH4_API(int, Recv_init, void *, int, MPI_Datatype, int, int, MPID_Comm *, int,
              MPID_Request **);
MPIDI_CH4_API(void, Request_set_completed, MPID_Request *);
MPIDI_CH4_API(void, Request_release, MPID_Request *);
MPIDI_CH4_API(int, Request_complete, MPID_Request *);
MPIDI_CH4_API(int, Request_is_anysource, MPID_Request *);
MPIDI_CH4_API(MPID_Request *, Request_create, void);
MPIDI_CH4_API(int, Send, const void *, int, MPI_Datatype, int, int, MPID_Comm *, int,
              MPID_Request **);
MPIDI_CH4_API(int, Ssend, const void *, int, MPI_Datatype, int, int, MPID_Comm *, int,
              MPID_Request **);
MPIDI_CH4_API(int, Rsend, const void *, int, MPI_Datatype, int, int, MPID_Comm *, int,
              MPID_Request **);
MPIDI_CH4_API(int, Irsend, const void *, int, MPI_Datatype, int, int, MPID_Comm *, int,
              MPID_Request **);
MPIDI_CH4_API(int, Send_init, const void *, int, MPI_Datatype, int, int, MPID_Comm *, int,
              MPID_Request **);
MPIDI_CH4_API(int, Ssend_init, const void *, int, MPI_Datatype, int, int, MPID_Comm *, int,
              MPID_Request **);
MPIDI_CH4_API(int, Bsend_init, const void *, int, MPI_Datatype, int, int, MPID_Comm *, int,
              MPID_Request **);
MPIDI_CH4_API(int, Rsend_init, const void *, int, MPI_Datatype, int, int, MPID_Comm *, int,
              MPID_Request **);
MPIDI_CH4_API(int, Startall, int, MPID_Request *[]);
MPIDI_CH4_API(int, GPID_Get, MPID_Comm *, int, MPID_Gpid *);
MPIDI_CH4_API(int, Accumulate, const void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype,
              MPI_Op, MPID_Win *);
MPIDI_CH4_API(int, Win_create, void *, MPI_Aint, int, MPID_Info *, MPID_Comm *, MPID_Win **);
MPIDI_CH4_API(int, Win_fence, int, MPID_Win *);
MPIDI_CH4_API(int, Win_free, MPID_Win **);
MPIDI_CH4_API(int, Get, void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype, MPID_Win *);
MPIDI_CH4_API(int, Win_get_info, MPID_Win *, MPID_Info **);
MPIDI_CH4_API(int, Win_lock, int, int, int, MPID_Win *);
MPIDI_CH4_API(int, Win_unlock, int, MPID_Win *);
MPIDI_CH4_API(int, Win_start, MPID_Group *, int, MPID_Win *);
MPIDI_CH4_API(int, Win_complete, MPID_Win *);
MPIDI_CH4_API(int, Win_post, MPID_Group *, int, MPID_Win *);
MPIDI_CH4_API(int, Win_wait, MPID_Win *);
MPIDI_CH4_API(int, Win_test, MPID_Win *, int *);
MPIDI_CH4_API(int, Put, const void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype,
              MPID_Win *);
MPIDI_CH4_API(int, Win_set_info, MPID_Win *, MPID_Info *);
MPIDI_CH4_API(int, Comm_reenable_anysource, MPID_Comm *, MPID_Group **);
MPIDI_CH4_API(int, Comm_remote_group_failed, MPID_Comm *, MPID_Group **);
MPIDI_CH4_API(int, Comm_group_failed, MPID_Comm *, MPID_Group **);
MPIDI_CH4_API(int, Win_attach, MPID_Win *, void *, MPI_Aint);
MPIDI_CH4_API(int, Win_allocate_shared, MPI_Aint, int, MPID_Info *, MPID_Comm *, void **,
              MPID_Win **);
MPIDI_CH4_API(int, Rput, const void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype,
              MPID_Win *, MPID_Request **);
MPIDI_CH4_API(int, Win_flush_local, int, MPID_Win *);
MPIDI_CH4_API(int, Win_detach, MPID_Win *, const void *);
MPIDI_CH4_API(int, Compare_and_swap, const void *, const void *, void *, MPI_Datatype, int,
              MPI_Aint, MPID_Win *);
MPIDI_CH4_API(int, Raccumulate, const void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype,
              MPI_Op, MPID_Win *, MPID_Request **);
MPIDI_CH4_API(int, Rget_accumulate, const void *, int, MPI_Datatype, void *, int, MPI_Datatype, int,
              MPI_Aint, int, MPI_Datatype, MPI_Op, MPID_Win *, MPID_Request **);
MPIDI_CH4_API(int, Fetch_and_op, const void *, void *, MPI_Datatype, int, MPI_Aint, MPI_Op,
              MPID_Win *);
MPIDI_CH4_API(int, Win_shared_query, MPID_Win *, int, MPI_Aint *, int *, void *);
MPIDI_CH4_API(int, Win_allocate, MPI_Aint, int, MPID_Info *, MPID_Comm *, void *, MPID_Win **);
MPIDI_CH4_API(int, Win_flush, int, MPID_Win *);
MPIDI_CH4_API(int, Win_flush_local_all, MPID_Win *);
MPIDI_CH4_API(int, Win_unlock_all, MPID_Win *);
MPIDI_CH4_API(int, Win_create_dynamic, MPID_Info *, MPID_Comm *, MPID_Win **);
MPIDI_CH4_API(int, Rget, void *, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype, MPID_Win *,
              MPID_Request **);
MPIDI_CH4_API(int, Win_sync, MPID_Win *);
MPIDI_CH4_API(int, Win_flush_all, MPID_Win *);
MPIDI_CH4_API(int, Get_accumulate, const void *, int, MPI_Datatype, void *, int, MPI_Datatype, int,
              MPI_Aint, int, MPI_Datatype, MPI_Op, MPID_Win *);
MPIDI_CH4_API(int, Win_lock_all, int, MPID_Win *);
MPIDI_CH4_API(void *, Alloc_mem, size_t, MPID_Info *);
MPIDI_CH4_API(int, Free_mem, void *);
MPIDI_CH4_API(int, Get_node_id, MPID_Comm *, int rank, MPID_Node_id_t *);
MPIDI_CH4_API(int, Get_max_node_id, MPID_Comm *, MPID_Node_id_t *);
MPIDI_CH4_API(int, Request_is_pending_failure, MPID_Request *);
MPIDI_CH4_API(MPI_Aint, Aint_add, MPI_Aint, MPI_Aint);
MPIDI_CH4_API(MPI_Aint, Aint_diff, MPI_Aint, MPI_Aint);
MPIDI_CH4_API(int, GPID_GetAllInComm, MPID_Comm *, int, MPID_Gpid[], int *);
MPIDI_CH4_API(int, GPID_ToLpidArray, int, MPID_Gpid[], int[]);
MPIDI_CH4_API(int, Create_intercomm_from_lpids, MPID_Comm *, int, const int[]);
MPIDI_CH4_API(int, Comm_create, MPID_Comm *);
MPIDI_CH4_API(int, Comm_destroy, MPID_Comm *);


/* Include netmod prototypes */
#include <netmod.h>
#ifdef MPIDI_BUILD_CH4_SHM
#include "ch4_shm.h"
#endif

/* Declare request functions here so netmods can refer to
   them in the NETMOD_DIRECT mode */
#include "ch4_request.h"

/* Include netmod and shm implementations  */
/* Prototypes are split from impl to avoid */
/* circular dependencies                   */
#include <netmod_impl.h>
#ifdef MPIDI_BUILD_CH4_SHM
#include "ch4_shm_impl.h"
#endif

#include "ch4_init.h"
#include "ch4_probe.h"
#include "ch4_send.h"
#include "ch4_recv.h"
#include "ch4_comm.h"
#include "ch4_win.h"
#include "ch4_rma.h"
#include "ch4_progress.h"
#include "ch4_spawn.h"

#define MPID_Abort                       MPIDI_Abort
#define MPID_Accumulate                  MPIDI_Accumulate
#define MPID_Alloc_mem                   MPIDI_Alloc_mem
#define MPID_Bsend_init                  MPIDI_Bsend_init
#define MPID_Cancel_recv                 MPIDI_Cancel_recv
#define MPID_Cancel_send                 MPIDI_Cancel_send
#define MPID_Close_port                  MPIDI_Close_port
#define MPID_Comm_accept                 MPIDI_Comm_accept
#define MPID_Comm_connect                MPIDI_Comm_connect
#define MPID_Comm_disconnect             MPIDI_Comm_disconnect
#define MPID_Comm_group_failed           MPIDI_Comm_group_failed
#define MPID_Comm_reenable_anysource     MPIDI_Comm_reenable_anysource
#define MPID_Comm_remote_group_failed    MPIDI_Comm_remote_group_failed
#define MPID_Comm_spawn_multiple         MPIDI_Comm_spawn_multiple
#define MPID_Comm_failure_get_acked      MPIDI_Comm_failure_get_acked
#define MPID_Comm_get_all_failed_procs   MPIDI_Comm_get_all_failed_procs
#define MPID_Comm_revoke                 MPIDI_Comm_revoke
#define MPID_Comm_failure_ack            MPIDI_Comm_failure_ack
#define MPID_Comm_AS_enabled             MPIDI_Comm_AS_enabled
#define MPID_Comm_get_lpid               MPIDI_Comm_get_lpid
#define MPID_Compare_and_swap            MPIDI_Compare_and_swap
#define MPID_Fetch_and_op                MPIDI_Fetch_and_op
#define MPID_Finalize                    MPIDI_Finalize
#define MPID_Free_mem                    MPIDI_Free_mem
#define MPID_GPID_Get                    MPIDI_GPID_Get
#define MPID_Get                         MPIDI_Get
#define MPID_Get_accumulate              MPIDI_Get_accumulate
#define MPID_Get_processor_name          MPIDI_Get_processor_name
#define MPID_Get_universe_size           MPIDI_Get_universe_size
#define MPID_Improbe                     MPIDI_Improbe
#define MPID_Imrecv                      MPIDI_Imrecv
#define MPID_Init                        MPIDI_Init
#define MPID_InitCompleted               MPIDI_InitCompleted
#define MPID_Iprobe                      MPIDI_Iprobe
#define MPID_Irecv                       MPIDI_Irecv
#define MPID_Irsend                      MPIDI_Irsend
#define MPID_Isend                       MPIDI_Isend
#define MPID_Issend                      MPIDI_Issend
#define MPID_Mprobe                      MPIDI_Mprobe
#define MPID_Mrecv                       MPIDI_Mrecv
#define MPID_Open_port                   MPIDI_Open_port
#define MPID_Probe                       MPIDI_Probe
#define MPID_Progress_end                MPIDI_Progress_end
#define MPID_Progress_poke               MPIDI_Progress_poke
#define MPID_Progress_start              MPIDI_Progress_start
#define MPID_Progress_test               MPIDI_Progress_test
#define MPID_Progress_wait               MPIDI_Progress_wait
#define MPID_Progress_register           MPIDI_Progress_register
#define MPID_Progress_deregister         MPIDI_Progress_deregister
#define MPID_Progress_activate           MPIDI_Progress_activate
#define MPID_Progress_deactivate         MPIDI_Progress_deactivate
#define MPID_Put                         MPIDI_Put
#define MPID_Raccumulate                 MPIDI_Raccumulate
#define MPID_Recv                        MPIDI_Recv
#define MPID_Recv_init                   MPIDI_Recv_init
#define MPID_Request_create              MPIDI_Request_create
#define MPID_Request_release             MPIDI_Request_release
#define MPID_Request_complete            MPIDI_Request_complete
#define MPID_Request_is_anysource        MPIDI_Request_is_anysource
#define MPID_Request_set_completed       MPIDI_Request_set_completed
#define MPID_Rget                        MPIDI_Rget
#define MPID_Rget_accumulate             MPIDI_Rget_accumulate
#define MPID_Rput                        MPIDI_Rput
#define MPID_Rsend                       MPIDI_Rsend
#define MPID_Rsend_init                  MPIDI_Rsend_init
#define MPID_Send                        MPIDI_Send
#define MPID_Send_init                   MPIDI_Send_init
#define MPID_Ssend                       MPIDI_Ssend
#define MPID_Ssend_init                  MPIDI_Ssend_init
#define MPID_Startall                    MPIDI_Startall
#define MPID_Win_allocate                MPIDI_Win_allocate
#define MPID_Win_allocate_shared         MPIDI_Win_allocate_shared
#define MPID_Win_attach                  MPIDI_Win_attach
#define MPID_Win_complete                MPIDI_Win_complete
#define MPID_Win_create                  MPIDI_Win_create
#define MPID_Win_create_dynamic          MPIDI_Win_create_dynamic
#define MPID_Win_detach                  MPIDI_Win_detach
#define MPID_Win_fence                   MPIDI_Win_fence
#define MPID_Win_flush                   MPIDI_Win_flush
#define MPID_Win_flush_all               MPIDI_Win_flush_all
#define MPID_Win_flush_local             MPIDI_Win_flush_local
#define MPID_Win_flush_local_all         MPIDI_Win_flush_local_all
#define MPID_Win_free                    MPIDI_Win_free
#define MPID_Win_get_info                MPIDI_Win_get_info
#define MPID_Win_lock                    MPIDI_Win_lock
#define MPID_Win_lock_all                MPIDI_Win_lock_all
#define MPID_Win_post                    MPIDI_Win_post
#define MPID_Win_set_info                MPIDI_Win_set_info
#define MPID_Win_shared_query            MPIDI_Win_shared_query
#define MPID_Win_start                   MPIDI_Win_start
#define MPID_Win_sync                    MPIDI_Win_sync
#define MPID_Win_test                    MPIDI_Win_test
#define MPID_Win_unlock                  MPIDI_Win_unlock
#define MPID_Win_unlock_all              MPIDI_Win_unlock_all
#define MPID_Win_wait                    MPIDI_Win_wait
#define MPID_Get_node_id                 MPIDI_Get_node_id
#define MPID_Get_max_node_id             MPIDI_Get_max_node_id
#define MPID_Request_is_pending_failure  MPIDI_Request_is_pending_failure
#define MPID_Aint_add                    MPIDI_Aint_add
#define MPID_Aint_diff                   MPIDI_Aint_diff
#define MPID_GPID_GetAllInComm           MPIDI_GPID_GetAllInComm
#define MPID_GPID_ToLpidArray            MPIDI_GPID_ToLpidArray
#define MPID_Create_intercomm_from_lpids MPIDI_Create_intercomm_from_lpids
/* Variables */
#define MPID_Request_direct              MPIDI_Request_direct
#define MPID_Request_mem                 MPIDI_Request_mem
#define MPID_Comm_create                 MPIDI_Comm_create
#define MPID_Comm_destroy                MPIDI_Comm_destroy


#define MPIDI_MAX_NETMOD_STRING_LEN 64
extern MPID_Request MPIDI_Request_direct[];
extern MPIU_Object_alloc_t MPIDI_Request_mem;
extern int MPIDI_num_netmods;



#endif /* !defined(MPICH_MPIDCH4_H_INCLUDED) */
