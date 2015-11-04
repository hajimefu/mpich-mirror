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
#include "ch4_impl.h"


#define COMM_TO_INDEX(comm,rank) COMM_OFI(comm).vcrt->vcr_table[rank].addr_idx
#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
#define COMM_TO_EP(comm,rank)  COMM_OFI(comm).vcrt->vcr_table[rank].ep_idx
#define MPIDI_MAX_ENDPOINTS 256
#define MPIDI_MAX_ENDPOINTS_BITS 8
#define G_TXC_TAG(x) MPIDI_Global.ctx[x].tx_tag
#define G_TXC_RMA(x) MPIDI_Global.ctx[x].tx_rma
#define G_TXC_MSG(x) MPIDI_Global.ctx[x].tx_msg
#define G_TXC_CTR(x) MPIDI_Global.ctx[x].tx_ctr
#define G_RXC_TAG(x) MPIDI_Global.ctx[x].rx_tag
#define G_RXC_RMA(x) MPIDI_Global.ctx[x].rx_rma
#define G_RXC_MSG(x) MPIDI_Global.ctx[x].rx_msg
#define G_RXC_CTR(x) MPIDI_Global.ctx[x].rx_ctr
#else
#define COMM_TO_EP(comm,rank) 0
#define MPIDI_MAX_ENDPOINTS 0
#define MPIDI_MAX_ENDPOINTS_BITS 0
#define G_TXC_TAG(x) MPIDI_Global.ep
#define G_TXC_RMA(x) MPIDI_Global.ep
#define G_TXC_MSG(x) MPIDI_Global.ep
#define G_TXC_CTR(x) MPIDI_Global.ep
#define G_RXC_TAG(x) MPIDI_Global.ep
#define G_RXC_RMA(x) MPIDI_Global.ep
#define G_RXC_MSG(x) MPIDI_Global.ep
#define G_RXC_CTR(x) MPIDI_Global.ep
#endif
#ifdef MPIDI_USE_AV_TABLE
#define COMM_TO_PHYS(comm,rank)  ((fi_addr_t)(uintptr_t)COMM_TO_INDEX(comm,rank))
#define TO_PHYS(rank)            ((fi_addr_t)(uintptr_t)rank)
#else
#define COMM_TO_PHYS(comm,rank)  MPIDI_Addr_table->table[COMM_TO_INDEX(comm,rank)].dest
#define TO_PHYS(rank)            MPIDI_Addr_table->table[rank].dest
#endif
#define REQ_OFI(req,field) ((req)->dev.ch4.netmod.am_ofi.field)
#define COMM_OFI(comm)     ((comm)->dev.ch4.netmod.am_ofi)
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

#define MPIDI_NM_PROGRESS()						\
    ({								\
	mpi_errno = MPIDI_Progress_test();			\
	if (mpi_errno!=MPI_SUCCESS) MPIR_ERR_POP(mpi_errno);	\
    })

#define MPIDI_NM_PROGRESS_WHILE(cond)			\
    ({						\
	while (cond)				\
	    MPIDI_NM_PROGRESS();				\
    })

#define MPIU_CH4_OFI_ERR  MPIR_ERR_CHKANDJUMP4
#define FI_RC(FUNC,STR)					\
    do							\
    {							\
	MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);			\
	ssize_t _ret = FUNC;				\
	MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);			\
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
	    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX); \
	    _ret = FUNC;                                                \
	    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_THREAD_GLOBAL_ALLFUNC_MUTEX);  \
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
	    MPIDI_NM_PROGRESS();                                                 \
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
	    MPIDI_NM_PROGRESS();							\
	} while (_ret == -FI_EAGAIN);					\
    } while (0)

#define MPIDI_NM_PMI_RC_POP(FUNC,STR)				\
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
extern int  MPIDI_VCRT_Create(int size, struct MPIDI_VCRT **vcrt_ptr);
extern int  MPIDI_VCRT_Release(struct MPIDI_VCRT *vcrt);
extern void MPIDI_Map_create(void **map);
extern void MPIDI_Map_destroy(void *map);
extern void MPIDI_Map_set(void *_map, uint64_t id, void *val);
extern void MPIDI_Map_erase(void *_map, uint64_t id);
extern void *MPIDI_Map_lookup(void *_map, uint64_t id);
extern void MPIDI_Index_datatypes();
extern int  MPIR_Datatype_init_names(void);

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
