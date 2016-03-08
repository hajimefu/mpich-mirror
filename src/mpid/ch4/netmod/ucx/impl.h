#ifndef NETMOD_UCX_IMPL_H_INCLUDED
#define NETMOD_UCX_IMPL_H_INCLUDED

#include <mpidimpl.h>
#include "ucx_types.h"
#include "mpidch4r.h"
#include "ch4_impl.h"

#include <ucx_types.h>
#include <ucs/type/status.h>

#define MPIDI_CH4_NMI_UCX_COMM(comm)     ((comm)->dev.ch4.netmod.ucx)
#define MPIDI_CH4_NMI_UCX_REQ(req)       ((req)->dev.ch4.netmod.ucx)
#define COMM_TO_INDEX(comm,rank) MPIDI_CH4_NMI_UCX_COMM(comm).vept->vep_table[rank].addr_idx
#define MPIDI_CH4_NMI_UCX_COMM_TO_EP(comm,rank)    MPIDI_CH4_NMI_UCX_eps[COMM_TO_INDEX(comm,rank)]

static inline uint64_t MPIDI_CH4_NMI_UCX_init_tag(MPIU_Context_id_t contextid, int source, uint64_t tag)
{
    uint64_t ucp_tag = 0;
    ucp_tag = contextid;
    ucp_tag = (ucp_tag << MPIDI_CH4_NMI_UCX_SOURCE_SHIFT);
    ucp_tag |= source;
    ucp_tag = (ucp_tag << MPIDI_CH4_NMI_UCX_TAG_SHIFT);
    ucp_tag |= (MPIDI_CH4_NMI_UCX_TAG_MASK & tag);
    return ucp_tag;
}

#ifndef MPIR_TAG_ERROR_BIT
#define MPIR_TAG_ERROR_BIT (1 << 30)
#endif
#ifndef  MPIR_TAG_PROC_FAILURE_BIT
#define MPIR_TAG_PROC_FAILURE_BIT (1 << 29)
#endif

static inline uint64_t MPIDI_CH4_NMI_UCX_tag_mask(int mpi_tag, int src)
{
    uint64_t tag_mask;
    tag_mask = ~(MPIR_TAG_PROC_FAILURE_BIT | MPIR_TAG_ERROR_BIT);
    if (mpi_tag == MPI_ANY_TAG)
        tag_mask &= ~MPIDI_CH4_NMI_UCX_TAG_MASK;

    if (src == MPI_ANY_SOURCE)
        tag_mask &= ~(MPIDI_CH4_NMI_UCX_SOURCE_MASK);

    return tag_mask;
}

static inline uint64_t MPIDI_CH4_NMI_UCX_recv_tag(int mpi_tag, int src, MPIU_Context_id_t contextid)
{
    uint64_t ucp_tag = contextid;

    ucp_tag = (ucp_tag << MPIDI_CH4_NMI_UCX_SOURCE_SHIFT);
    if (src != MPI_ANY_SOURCE)
        ucp_tag |= (src & UCS_MASK(MPIDI_CH4_NMI_UCX_CONTEXT_RANK_BITS));
    ucp_tag = ucp_tag << MPIDI_CH4_NMI_UCX_TAG_SHIFT;
    if (mpi_tag != MPI_ANY_TAG)
        ucp_tag |= (MPIDI_CH4_NMI_UCX_TAG_MASK & mpi_tag);
    return ucp_tag;
}

static inline int MPIDI_CH4_NMI_UCX_get_tag(uint64_t match_bits)
{
    return ((int) (match_bits & MPIDI_CH4_NMI_UCX_TAG_MASK));
}

static inline int MPIDI_CH4_NMI_UCX_get_source(uint64_t match_bits)
{
    return ((int) ((match_bits & MPIDI_CH4_NMI_UCX_SOURCE_MASK) >> MPIDI_CH4_NMI_UCX_TAG_SHIFT));
}


#define MPIDI_CH4_NMI_UCX_ERR  MPIR_ERR_CHKANDJUMP4

#define MPIDI_CH4_NMI_UCX_CHK_STATUS(STATUS,STR)                \
  do {								\
    MPIDI_CH4_NMI_UCX_ERR(STATUS!=UCS_OK,			\
			  mpi_errno,				\
			  MPI_ERR_OTHER,			\
			  "**ch4_ucx_nm_"#STR,                  \
			  "**ch4_ucx_nm_"#STR" %s %d %s %s",    \
			  __SHORT_FILE__,			\
			  __LINE__,				\
			  FCNAME,				\
			  ucs_status_string(STATUS));		\
    } while (0)



#define MPIDI_CH4_NMI_UCX_PMI_ERROR(_errno,STR)				\
  do									\
    {									\
      MPIDI_CH4_NMI_UCX_ERR(_errno!=PMI_SUCCESS,			\
			    mpi_errno,					\
			    MPI_ERR_OTHER,				\
			    "**ch4_ucx_nm_pmi"#STR,			\
			    "**ch4_ucx_nm_mpi"#STR" %s %d %s %s",	\
			    __SHORT_FILE__,				\
			    __LINE__,					\
			    FCNAME,					\
			    #STR);					\
    } while (0)

#define MPIDI_CH4_UCX_MPI_ERROR(_errno)				     \
  do								     \
    {								     \
      if (unlikely(_errno!=MPI_SUCCESS)) MPIR_ERR_POP(mpi_errno);    \
    } while (0)

#define MPIDI_CH4_UCX_STR_ERRCHK(_errno,STR)				\
  do									\
    {									\
      MPIDI_CH4_NMI_UCX_ERR(_errno!=MPL_STR_SUCCESS,			\
			    mpi_errno,					\
			    MPI_ERR_OTHER,				\
			    "**ch4_ucx_nm_"#STR,			\
			    "**ch4_ucx_nm_"#STR" %s %d %s %s",		\
			    __SHORT_FILE__,				\
			    __LINE__,					\
			    FCNAME,					\
			    #STR);					\
    } while (0)



#define MPIDI_CH4_UCX_REQUEST(_req, STR)				\
  do {									\
    MPIDI_CH4_NMI_UCX_ERR(UCS_PTR_IS_ERR(_req),				\
			  mpi_errno,					\
			  MPI_ERR_OTHER,				\
			  "**ch4_ucx_nm_"#STR,				\
			  "**ch4_ucx_nm_"#STR" %s %d %s %s",		\
			  __SHORT_FILE__,				\
			  __LINE__,					\
			  FCNAME,					\
			  ucs_status_string(UCS_PTR_STATUS(_req)));	\
  } while (0)

#define MPIDI_Request_init_sreq(sreq_)	\
{								\
    (sreq_)->kind = MPID_REQUEST_SEND;	  \
    (sreq_)->comm = comm;		          \
    (sreq_)->partner_request   = NULL;    \
    MPIR_Comm_add_ref(comm);		\
    MPIR_cc_set(&sreq_->cc, 1);     \
    req->cc_ptr = &req->cc;         \
    MPIU_Object_set_ref(req, 2);    \
    req->greq_fns = NULL;       \
    MPIR_STATUS_SET_COUNT(req->status, 0);\
    MPIR_STATUS_SET_CANCEL_BIT(req->status, FALSE); \
    req->status.MPI_SOURCE = MPI_UNDEFINED;\
    req->status.MPI_TAG = MPI_UNDEFINED;\
    req->status.MPI_ERROR = MPI_SUCCESS;\
    req->errflag = MPIR_ERR_NONE;\
}

#define MPIDI_Request_init_rreq(rreq_)	\
{								\
    (rreq_)->kind = MPID_REQUEST_RECV;	    \
    (rreq_)->partner_request   = NULL;      \
    MPIR_cc_set(&rreq_->cc, 1);             \
    rreq_->cc_ptr = &rreq_->cc;             \
    MPIU_Object_set_ref((rreq_), 2);        \
    (rreq_)->greq_fns = NULL;               \
    MPIR_STATUS_SET_COUNT(rreq_->status, 0);\
    (rreq_)->status.MPI_ERROR = MPI_SUCCESS;\
    (rreq_)->errflag = MPIR_ERR_NONE;\
}

int MPIDI_CH4_NMI_UCX_VEPT_Create(int size, struct MPIDI_VEPT **vept_ptr);
extern int MPIR_Datatype_init_names(void);
#endif
