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
#ifndef NETMOD_AM_OFI_IMPL_H_INCLUDED
#define NETMOD_AM_OFI_IMPL_H_INCLUDED

#include <mpidimpl.h>
#include "types.h"
#include "mpidch4u.h"
#include "../../src/ch4_impl.h"

/*
 * Helper routines and macros for request completion
 */
#define MPIDI_Request_tls_alloc(req)					\
    ({									\
	(req) = (MPID_Request*)MPIU_Handle_obj_alloc(&MPIDI_Request_mem); \
	if (req == NULL)						\
	    MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Request"); \
    })

#define MPIDI_Request_tls_free(req)			\
    MPIU_Handle_obj_free(&MPIDI_Request_mem, (req))

#define MPIDI_Request_alloc_and_init(req,count)		\
    ({							\
	MPIDI_Request_tls_alloc(req);			\
	MPIU_Assert(req != NULL);			\
	MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)	\
		    == MPID_REQUEST);			\
	MPID_cc_set(&req->cc, 1);			\
	req->cc_ptr = &req->cc;				\
	MPIU_Object_set_ref(req, count);		\
	req->greq_fns          = NULL;			\
	MPIR_STATUS_SET_COUNT(req->status, 0);		\
	MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE);	\
	req->status.MPI_SOURCE    = MPI_UNDEFINED;	\
	req->status.MPI_TAG       = MPI_UNDEFINED;	\
	req->status.MPI_ERROR     = MPI_SUCCESS;	\
	req->comm                 = NULL;		\
    })

#define MPIDI_Win_request_tls_alloc(req)				\
    ({									\
	(req) = (MPIDI_Win_request*)MPIU_Handle_obj_alloc(&MPIDI_Request_mem); \
	if (req == NULL)						\
	    MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1, "Cannot allocate Win Request"); \
    })

#define MPIDI_Win_request_tls_free(req)			\
    MPIU_Handle_obj_free(&MPIDI_Request_mem, (req))

#define MPIDI_Win_request_complete(req)                 \
    ({							\
	int count;					\
	MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)	\
		    == MPID_REQUEST);			\
	MPIU_Object_release_ref(req, &count);		\
	MPIU_Assert(count >= 0);			\
	if (count == 0)					\
	{						\
	    MPIU_Free(req->noncontig);			\
	    MPIDI_Win_request_tls_free(req);		\
	}						\
    })

#define MPIDI_Win_request_alloc_and_init(req,count)			\
    ({									\
	MPIDI_Win_request_tls_alloc(req);				\
	MPIU_Assert(req != NULL);					\
	MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)			\
		    == MPID_REQUEST);					\
	MPIU_Object_set_ref(req, count);				\
	memset((char*)req+MPIDI_REQUEST_HDR_SIZE, 0,			\
	       sizeof(MPIDI_Win_request)-				\
	       MPIDI_REQUEST_HDR_SIZE);					\
	req->noncontig = (MPIDI_Win_noncontig*)MPIU_Calloc(1,sizeof(*(req->noncontig))); \
    })

#define MPIDI_Ssendack_request_tls_alloc(req)			\
    ({								\
	(req) = (MPIDI_Ssendack_request*)			\
	    MPIU_Handle_obj_alloc(&MPIDI_Request_mem);		\
	if (req == NULL)					\
	    MPID_Abort(NULL, MPI_ERR_NO_SPACE, -1,		\
		       "Cannot allocate Ssendack Request");	\
    })

#define MPIDI_Ssendack_request_tls_free(req)		\
    MPIU_Handle_obj_free(&MPIDI_Request_mem, (req))

#define MPIDI_Ssendack_request_alloc_and_init(req)      \
    ({							\
	MPIDI_Ssendack_request_tls_alloc(req);		\
	MPIU_Assert(req != NULL);			\
	MPIU_Assert(HANDLE_GET_MPI_KIND(req->handle)	\
		    == MPID_SSENDACK_REQUEST);		\
    })

#define PROGRESS()						\
    ({								\
	mpi_errno = MPIDI_Progress_test();			\
	if (mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);	\
    })

#define PROGRESS_WHILE(cond)			\
    ({						\
	while (cond)				\
	    PROGRESS();				\
    })

#define MPIU_CH4_OFI_ERR  MPIR_ERR_CHKANDJUMP4
#define FI_RC(FUNC,STR)					\
    do							\
    {							\
	MPID_THREAD_CS_ENTER(GLOBAL, MPIR_ThreadInfo.global_mutex);			\
	ssize_t _ret = FUNC;				\
	MPID_THREAD_CS_EXIT(GLOBAL, MPIR_ThreadInfo.global_mutex);			\
	MPIU_CH4_OFI_ERR(_ret<0,			\
			 mpi_errno,			\
			 MPI_ERR_OTHER,			\
			 "**ofid_"#STR,			\
			 "**ofid_"#STR" %s %d %s %s",	\
			 __SHORT_FILE__,		\
			 __LINE__,			\
			 FCNAME,			\
			 fi_strerror(-_ret));		\
    } while (0)

#define FI_RC_RETRY(FUNC,STR)                                           \
    do                                                                  \
    {                                                                   \
	ssize_t _ret;                                                   \
	do {                                                            \
	    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_ThreadInfo.global_mutex); \
	    _ret = FUNC;                                                \
	    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_ThreadInfo.global_mutex);  \
	    if (likely(_ret==0)) break;                                 \
	    MPIU_CH4_OFI_ERR(_ret != -FI_EAGAIN,                        \
			     mpi_errno,                                 \
			     MPI_ERR_OTHER,                             \
			     "**ofid_"#STR,                             \
			     "**ofid_"#STR" %s %d %s %s",               \
			     __SHORT_FILE__,                            \
			     __LINE__,                                  \
			     FCNAME,                                    \
			     fi_strerror(-_ret));                       \
	    PROGRESS();                                                 \
	} while (_ret == -FI_EAGAIN);                                   \
    } while (0)

#define FI_RC_RETRY_NOLOCK(FUNC,STR)					\
    do									\
    {									\
	ssize_t _ret;							\
	do {								\
	    _ret = FUNC;						\
	    if (likely(_ret==0)) break;					\
	    MPIU_CH4_OFI_ERR(_ret != -FI_EAGAIN,                        \
			     mpi_errno,					\
			     MPI_ERR_OTHER,				\
			     "**ofid_"#STR,				\
			     "**ofid_"#STR" %s %d %s %s",		\
			     __SHORT_FILE__,				\
			     __LINE__,					\
			     FCNAME,					\
			     fi_strerror(-_ret));			\
	    PROGRESS();							\
	} while (_ret == -FI_EAGAIN);					\
    } while (0)

#define PMI_RC(FUNC,STR)				\
    do							\
    {							\
	pmi_errno  = FUNC;				\
	MPIU_CH4_OFI_ERR(pmi_errno!=PMI_SUCCESS,	\
			 mpi_errno,			\
			 MPI_ERR_OTHER,			\
			 "**ofid_"#STR,			\
			 "**ofid_"#STR" %s %d %s %s",	\
			 __SHORT_FILE__,		\
			 __LINE__,			\
			 FCNAME,			\
			 #STR);				\
    } while (0)

#define MPI_RC_POP(FUNC)					\
    do								\
    {								\
	mpi_errno = FUNC;					\
	if (mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);	\
    } while (0)

#define MPIU_STR_RC(FUNC,STR)				\
    do							\
    {							\
	str_errno = FUNC;				\
	MPIU_CH4_OFI_ERR(str_errno!=MPIU_STR_SUCCESS,	\
			 mpi_errno,			\
			 MPI_ERR_OTHER,			\
			 "**"#STR,			\
			 "**"#STR" %s %d %s %s",	\
			 __SHORT_FILE__,		\
			 __LINE__,			\
			 FCNAME,			\
			 #STR);				\
    } while (0)

#define REQ_CREATE(req)					\
	({						\
	    MPIDI_Request_alloc_and_init(req,2);	\
	})


#define WINREQ_CREATE(req)				\
	({						\
	    MPIDI_Win_request_alloc_and_init(req,1);	\
	})

#define SSENDACKREQ_CREATE(req)				\
	({						\
	    MPIDI_Ssendack_request_tls_alloc(req);	\
	})

#define MPID_LKEY_START 16384

extern MPIDI_Addr_table_t *MPIDI_Addr_table;
extern MPIDI_Global_t MPIDI_Global;
extern MPIU_Object_alloc_t MPIDI_Request_mem;
extern MPID_Request MPIDI_Request_direct[];

/* Utility functions */
extern int MPIDI_VCRT_Create(int size, struct MPIDI_VCRT **vcrt_ptr);
extern int MPIDI_VCRT_Release(struct MPIDI_VCRT *vcrt);
extern void MPIDI_Map_create(void **map);
extern void MPIDI_Map_destroy(void *map);
extern void MPIDI_Map_set(void *_map, uint64_t id, void *val);
extern void MPIDI_Map_erase(void *_map, uint64_t id);
extern void *MPIDI_Map_lookup(void *_map, uint64_t id);

extern void MPIDI_build_nodemap(uint32_t * in_nodeids,
                                MPID_Node_id_t * out_nodemap, int sz, MPID_Node_id_t * sz_out);
extern void MPIDI_Index_datatypes();

/* Prototypes for inliner */
extern int MPIR_Datatype_init_names(void);
extern int MPIR_Allgather_impl(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                               void *recvbuf, int recvcount, MPI_Datatype recvtype,
                               MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag);
extern int MPIR_Barrier_impl(MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag);
extern int MPIR_Comm_commit(MPID_Comm *);
extern int MPIR_Comm_split_impl(MPID_Comm * comm_ptr, int color, int key, MPID_Comm ** newcomm_ptr);
extern int MPIR_Allreduce_impl(const void *sendbuf, void *recvbuf, int count,
                               MPI_Datatype datatype, MPI_Op op, MPID_Comm * comm_ptr,
                               MPIR_Errflag_t * errflag);
extern int MPIR_Bcast_impl(void *buffer, int count, MPI_Datatype datatype, int root,
                           MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag);
extern int MPIR_Localcopy(const void *sendbuf, MPI_Aint sendcount, MPI_Datatype sendtype,
                          void *recvbuf, MPI_Aint recvcount, MPI_Datatype recvtype);
extern int MPIR_Info_set_impl(MPID_Info * info_ptr, const char *key, const char *value);
extern int MPIR_Bcast_intra(void *buffer, int count, MPI_Datatype datatype, int
                            root, MPID_Comm * comm_ptr, MPIR_Errflag_t * errflag);
extern int MPIDU_Sched_progress(int *made_progress);
extern int MPIR_Comm_create(MPID_Comm **);
extern int MPIR_Comm_dup_impl(MPID_Comm * comm_ptr, MPID_Comm ** newcomm_ptr);
extern int MPIR_Comm_free_impl(MPID_Comm * comm_ptr);

/* Common Utility functions used by the
 * C and C++ components
 */
static inline fi_addr_t _comm_to_phys(MPID_Comm * comm, int rank, int ep_family)
{
    return COMM_TO_PHYS(comm, rank);
}

static inline fi_addr_t _to_phys(int rank, int ep_family)
{
    return TO_PHYS(rank);
}

EXTERN_C_END
#endif /* NETMOD_AM_OFI_IMPL_H_INCLUDED */
