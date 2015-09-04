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
#ifndef NETMOD_AM_OFI_INIT_H_INCLUDED
#define NETMOD_AM_OFI_INIT_H_INCLUDED

#include <mpidimpl.h>
#include "mpich_cvars.h"
#include "impl.h"
#include "pmi.h"

#define CHOOSE_PROVIDER(prov, prov_use,errstr)                          \
    do {								\
	struct fi_info *p = prov;					\
	MPIR_ERR_CHKANDJUMP4(p==NULL, mpi_errno,MPI_ERR_OTHER,"**ofid_addrinfo", \
			     "**ofid_addrinfo %s %d %s %s",__SHORT_FILE__, \
			     __LINE__,FCNAME, errstr);			\
	MPIDI_Choose_provider(prov,prov_use);				\
    } while (0)

static inline int MPIDI_Choose_provider(struct fi_info *prov, struct fi_info **prov_use);
static inline int MPIDI_Create_endpoint(struct fi_info *prov_use,
                                        struct fid_domain *domain,
                                        struct fid_cq *cq,
                                        struct fid_mr *mr, struct fid_av *av, struct fid_ep **ep);

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_init(int rank, int size, int appnum, int *tag_ub,
                                    MPID_Comm * comm_world, MPID_Comm * comm_self,
                                    int spawned,int num_contexts, void **netmod_contexts)
{
    int mpi_errno = MPI_SUCCESS, pmi_errno;
    int str_errno, maxlen, i, fi_version;
    char *table = NULL, *provname = NULL;
    MPID_Comm *comm;
    struct fi_info *hints, *prov, *prov_use;
    uint64_t mr_flags;
    struct fi_cq_attr cq_attr;
    fi_addr_t *mapped_table;
    struct fi_av_attr av_attr;
    char valS[MPIDI_KVSAPPSTRLEN], *val;
    char keyS[MPIDI_KVSAPPSTRLEN];
    size_t optlen;

    CH4_COMPILE_TIME_ASSERT(sizeof(MPID_Request) >=
                            (offsetof(MPID_Request, dev.ch4.ch4u.netmod_am) +
                             sizeof(MPIDI_netmod_am_ofi_amrequest_t)));

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_AM_OFI_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_AM_OFI_INIT);

    hints = fi_allocinfo();
    MPIU_Assert(hints != NULL);

    hints->mode = FI_CONTEXT;
    hints->caps = FI_MSG | FI_MULTI_RECV | FI_RMA | FI_SOURCE;
    fi_version = FI_VERSION(MPIDI_FI_MAJOR_VERSION, MPIDI_FI_MINOR_VERSION);

    hints->addr_format = FI_FORMAT_UNSPEC;
    hints->domain_attr->threading = FI_THREAD_ENDPOINT;
    hints->domain_attr->control_progress = FI_PROGRESS_AUTO;
    hints->domain_attr->data_progress = FI_PROGRESS_AUTO;
    hints->domain_attr->resource_mgmt = FI_RM_ENABLED;
    hints->domain_attr->av_type = FI_AV_UNSPEC;
    hints->domain_attr->mr_mode = FI_MR_SCALABLE;
    hints->tx_attr->op_flags = FI_DELIVERY_COMPLETE | FI_COMPLETION;
    hints->rx_attr->op_flags = FI_COMPLETION;
    hints->ep_attr->type = FI_EP_RDM;

    provname = MPIR_CVAR_OFI_USE_PROVIDER ? (char *) MPIU_Strdup(MPIR_CVAR_OFI_USE_PROVIDER) : NULL;
    hints->fabric_attr->prov_name = provname;
    FI_RC(fi_getinfo(fi_version, NULL, NULL, 0ULL, hints, &prov), addrinfo);
    CHOOSE_PROVIDER(prov, &prov_use, "No suitable provider provider found");

    MPIDI_Global.max_buffered_send = prov_use->tx_attr->inject_size;
    MPIDI_Global.max_send = prov_use->ep_attr->max_msg_size;
    MPIDI_Global.max_short_send = MPIDI_MAX_SHORT_SEND_SZ;

    FI_RC(fi_fabric(prov_use->fabric_attr, &MPIDI_Global.fabric, NULL), fabric);
    FI_RC(fi_domain(MPIDI_Global.fabric, prov_use, &MPIDI_Global.domain, NULL), opendomain);

#ifdef MPIDI_USE_MR_OFFSET
    mr_flags = FI_MR_OFFSET;
#else
    mr_flags = 0ULL;
#endif

    FI_RC(fi_mr_reg(MPIDI_Global.domain, 0, UINTPTR_MAX,
                    FI_REMOTE_READ | FI_SEND | FI_RECV, 0ULL, 0ULL,
                    mr_flags, &MPIDI_Global.mr, NULL), mr_reg);
    MPIDI_Global.lkey = fi_mr_key(MPIDI_Global.mr);
    MPIU_Assert(MPIDI_Global.lkey == 0);

    memset(&cq_attr, 0, sizeof(cq_attr));
    cq_attr.format = FI_CQ_FORMAT_DATA;
    FI_RC(fi_cq_open(MPIDI_Global.domain, &cq_attr, &MPIDI_Global.am_cq, NULL), opencq);

    memset(&av_attr, 0, sizeof(av_attr));
#ifdef MPIDI_USE_AV_TABLE
    av_attr.type = FI_AV_TABLE;
    MPIDI_Addr_table = (MPIDI_Addr_table_t *) MPIU_Malloc(sizeof(MPIDI_Addr_table_t));
    MPIDI_Addr_table->size = size;
    mapped_table = NULL;
#else
    av_attr.type = FI_AV_MAP;
    MPIDI_Addr_table = (MPIDI_Addr_table_t *) MPIU_Malloc(size * sizeof(fi_addr_t) +
                                                          sizeof(MPIDI_Addr_table_t));
    MPIDI_Addr_table->size = size;
    mapped_table = (fi_addr_t *) MPIDI_Addr_table->table;
#endif

    FI_RC(fi_av_open(MPIDI_Global.domain, &av_attr, &MPIDI_Global.av, NULL), avopen);

    MPIDU_RC_POP(MPIDI_Create_endpoint(prov_use, MPIDI_Global.domain, MPIDI_Global.am_cq,
                                     MPIDI_Global.mr, MPIDI_Global.av, &MPIDI_Global.ep));

    MPIDI_Global.addrnamelen = FI_NAME_MAX;
    FI_RC(fi_getname((fid_t) MPIDI_Global.ep, MPIDI_Global.addrname,
                     &MPIDI_Global.addrnamelen), getname);
    MPIU_Assert(MPIDI_Global.addrnamelen <= FI_NAME_MAX);

    val = valS;
    str_errno = MPIU_STR_SUCCESS;
    maxlen = MPIDI_KVSAPPSTRLEN;
    memset(val, 0, maxlen);
    MPIU_STR_RC(MPIU_Str_add_binary_arg(&val, &maxlen, "OFI",
                                        (char *) &MPIDI_Global.addrname,
                                        MPIDI_Global.addrnamelen), buscard_len);
    PMI_RC(PMI_KVS_Get_my_name(MPIDI_Global.kvsname, MPIDI_KVSAPPSTRLEN), pmi);

    val = valS;
    sprintf(keyS, "OFI-%d", rank);
    PMI_RC(PMI_KVS_Put(MPIDI_Global.kvsname, keyS, val), pmi);
    PMI_RC(PMI_KVS_Commit(MPIDI_Global.kvsname), pmi);
    PMI_RC(PMI_Barrier(), pmi);

    table = (char *) MPIU_Malloc(size * MPIDI_Global.addrnamelen);
    maxlen = MPIDI_KVSAPPSTRLEN;

    for (i = 0; i < size; i++) {
        sprintf(keyS, "OFI-%d", i);
        PMI_RC(PMI_KVS_Get(MPIDI_Global.kvsname, keyS, valS, MPIDI_KVSAPPSTRLEN), pmi);
        MPIU_STR_RC(MPIU_Str_get_binary_arg(valS, "OFI",
                                            (char *) &table[i * MPIDI_Global.addrnamelen],
                                            MPIDI_Global.addrnamelen, &maxlen), buscard_len);
    }

    FI_RC(fi_av_insert(MPIDI_Global.av, table, size, mapped_table, 0ULL, NULL), avmap);
    MPIDI_Map_create(&MPIDI_Global.win_map);
    MPIDI_Map_create(&MPIDI_Global.comm_map);

    comm = MPIR_Process.comm_self;
    comm->rank = 0;
    comm->remote_size = 1;
    comm->local_size = 1;
    MPIDU_RC_POP(MPIDI_VCRT_Create(comm->remote_size, &COMM_OFI(comm).vcrt));
    COMM_OFI(comm).vcrt->vcr_table[0].addr_idx = rank;

    comm = MPIR_Process.comm_world;
    comm->rank = rank;
    comm->remote_size = size;
    comm->local_size = size;
    MPIDU_RC_POP(MPIDI_VCRT_Create(comm->remote_size, &COMM_OFI(comm).vcrt));

    MPIDU_RC_POP(MPIDI_CH4U_init(comm_world, comm_self, num_contexts, netmod_contexts));

    optlen = MPIDI_MIN_MSG_SZ;
    FI_RC(fi_setopt(&MPIDI_Global.ep->fid, FI_OPT_ENDPOINT,
                    FI_OPT_MIN_MULTI_RECV, &optlen, sizeof(optlen)), setopt);

    for (i = 0; i < MPIDI_NUM_AM_BUFFERS; i++) {
        MPIDI_Global.am_bufs[i] = MPIU_Malloc(MPIDI_AM_BUFF_SZ);
        MPIU_Assert(MPIDI_Global.am_bufs[i]);

        MPIDI_Global.am_iov[i].iov_base = MPIDI_Global.am_bufs[i];
        MPIDI_Global.am_iov[i].iov_len = MPIDI_AM_BUFF_SZ;

        MPIDI_Global.am_msg[i].msg_iov = &MPIDI_Global.am_iov[i];
        MPIDI_Global.am_msg[i].desc = NULL;
        MPIDI_Global.am_msg[i].addr = FI_ADDR_UNSPEC;
        MPIDI_Global.am_msg[i].context = &MPIDI_Global.am_msg[i];
        MPIDI_Global.am_msg[i].iov_count = 1;
        FI_RC_RETRY(fi_recvmsg(MPIDI_Global.ep, &MPIDI_Global.am_msg[i], FI_MULTI_RECV), prepost);
    }

    MPIDI_Global.node_map = (MPID_Node_id_t *)
        MPIU_Malloc(comm_world->local_size*sizeof(*MPIDI_Global.node_map));
    MPIDI_CH4U_build_nodemap(comm_world->rank,
                             comm_world,
                             comm_world->local_size,
                             MPIDI_Global.node_map,
                             &MPIDI_Global.max_node_id);

    MPIR_Datatype_init_names();
    /* todo: spawn */

  fn_exit:
    if (provname) {
        MPIU_Free(provname);
        hints->fabric_attr->prov_name = NULL;
    }
    if (prov)
        fi_freeinfo(prov);
    fi_freeinfo(hints);
    if (table)
        MPIU_Free(table);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_AM_OFI_INIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_netmod_finalize(void)
{
    int mpi_errno = MPI_SUCCESS;
    int i = 0;
    int barrier[2] = { 0 };
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPID_Comm *comm;

    /* Barrier over allreduce, but force non-immediate send */
    MPIDI_Global.max_buffered_send = 0;
    MPIDU_RC_POP(MPIR_Allreduce_impl(&barrier[0], &barrier[1], 1, MPI_INT,
                                   MPI_SUM, MPIR_Process.comm_world, &errflag));

    FI_RC(fi_close(&MPIDI_Global.mr->fid), mr_unreg);
    FI_RC(fi_close(&MPIDI_Global.ep->fid), epclose);
    FI_RC(fi_close(&MPIDI_Global.av->fid), avclose);
    FI_RC(fi_close(&MPIDI_Global.am_cq->fid), cqclose);
    FI_RC(fi_close(&MPIDI_Global.domain->fid), domainclose);

    for (i = 0; i < MPIDI_NUM_AM_BUFFERS; i++) {
        MPIU_Free(MPIDI_Global.am_bufs[i]);
    }

    comm = MPIR_Process.comm_world;
    MPIR_Comm_release_always(comm);

    comm = MPIR_Process.comm_self;
    MPIR_Comm_release_always(comm);

    MPIU_Free(MPIDI_Addr_table);
    MPIU_Free(MPIDI_Global.node_map);

    MPIDI_Map_destroy(MPIDI_Global.win_map);
    MPIDI_Map_destroy(MPIDI_Global.comm_map);

    PMI_Finalize();

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


static inline int MPIDI_netmod_comm_get_lpid(MPID_Comm * comm_ptr,
                                             int idx, int *lpid_ptr, MPIU_BOOL is_remote)
{
    if (comm_ptr->comm_kind == MPID_INTRACOMM)
        *lpid_ptr = COMM_TO_INDEX(comm_ptr, idx);
    else if (is_remote)
        *lpid_ptr = COMM_TO_INDEX(comm_ptr, idx);
    else
        *lpid_ptr = COMM_OFI(comm_ptr).local_vcrt->vcr_table[idx].addr_idx;

    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_gpid_get(MPID_Comm * comm_ptr, int rank, MPID_Gpid * gpid)
{
    int mpi_errno = MPI_SUCCESS;
    MPIU_Assert(rank < comm_ptr->local_size);
    size_t sz = sizeof(GPID_OFI(gpid)->addr);
    FI_RC(fi_av_lookup(MPIDI_Global.av, COMM_TO_PHYS(comm_ptr, rank),
                       &GPID_OFI(gpid)->addr, &sz), avlookup);
    MPIU_Assert(sz <= sizeof(GPID_OFI(gpid)->addr));
    GPID_OFI(gpid)->node = MPIDI_Global.node_map[COMM_TO_INDEX(comm_ptr, rank)];
  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_netmod_get_node_id(MPID_Comm * comm, int rank, MPID_Node_id_t * id_p)
{
    *id_p = MPIDI_Global.node_map[COMM_TO_INDEX(comm, rank)];
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_get_max_node_id(MPID_Comm * comm, MPID_Node_id_t * max_id_p)
{
    *max_id_p = MPIDI_Global.max_node_id;
    return MPI_SUCCESS;
}

static inline int MPIDI_netmod_getallincomm(MPID_Comm * comm_ptr,
                                            int local_size, MPID_Gpid local_gpids[], int *singlePG)
{
    int i;

    for (i = 0; i < comm_ptr->local_size; i++)
        MPIDI_GPID_Get(comm_ptr, i, &local_gpids[i]);

    *singlePG = 0;
    return 0;
}

static inline int MPIDI_netmod_gpid_tolpidarray(int size, MPID_Gpid gpid[], int lpid[])
{
    int i, mpi_errno = MPI_SUCCESS;

    for (i = 0; i < size; i++) {
        int j;
        char tbladdr[FI_NAME_MAX];
        int found = 0;

        for (j = 0; j < MPIDI_Addr_table->size; j++) {
            size_t sz = sizeof(GPID_OFI(&gpid[i])->addr);
            FI_RC(fi_av_lookup(MPIDI_Global.av, TO_PHYS(j), &tbladdr, &sz), avlookup);
            MPIU_Assert(sz <= sizeof(GPID_OFI(&gpid[i])->addr));

            if (!memcmp(tbladdr, GPID_OFI(&gpid[i])->addr, sz)) {
                lpid[i] = j;
                found = 1;
                break;
            }
        }

        if (!found) {
            int start = MPIDI_Addr_table->size;
            fi_addr_t addr;
            MPIDI_Global.node_map = (MPID_Node_id_t *) MPIU_Realloc(MPIDI_Global.node_map,
                                                                    (1 +
                                                                     start) *
                                                                    sizeof(MPID_Node_id_t));
            MPIDI_Global.node_map[start] = GPID_OFI(&gpid[i])->node;
#ifndef MPIDI_USE_AV_TABLE
            MPIDI_Addr_table = (MPIDI_Addr_table_t *) MPIU_Realloc(MPIDI_Addr_table,
                                                                   (1 + start) * sizeof(fi_addr_t) +
                                                                   sizeof(MPIDI_Addr_table_t));
#endif
            addr = TO_PHYS(start);
            FI_RC(fi_av_insert(MPIDI_Global.av, &GPID_OFI(&gpid[i])->addr,
                               1, &addr, 0ULL, NULL), avmap);
            MPIDI_Addr_table->size++;
            lpid[i] = start;
        }
    }

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_netmod_create_intercomm_from_lpids(MPID_Comm * newcomm_ptr,
                                                           int size, const int lpids[])
{
    int i;
    MPIDI_VCRT_Create(size, &COMM_OFI(newcomm_ptr).vcrt);

    for (i = 0; i < size; i++)
        COMM_OFI(newcomm_ptr).vcrt->vcr_table[i].addr_idx = lpids[i];

    return 0;
}

static inline int MPIDI_netmod_free_mem(void *ptr)
{
    return MPIDI_CH4U_free_mem(ptr);
}

static inline void *MPIDI_netmod_alloc_mem(size_t size, MPID_Info * info_ptr)
{
    return MPIDI_CH4U_alloc_mem(size, info_ptr);
}

#undef FUNCNAME
#define FUNCNAME MPIDI_Create_endpoint
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_Create_endpoint(struct fi_info *prov_use,
                                        struct fid_domain *domain,
                                        struct fid_cq *cq, struct fid_mr *mr,
                                        struct fid_av *av, struct fid_ep **ep)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_CREATE_ENDPOINT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_CREATE_ENDPOINT);

    FI_RC(fi_endpoint(domain, prov_use, ep, NULL), ep);
    FI_RC(fi_ep_bind(*ep, &cq->fid, FI_SEND | FI_RECV | FI_WRITE | FI_READ), cq_bind);
    FI_RC(fi_ep_bind(*ep, &av->fid, 0), av_bind);
    FI_RC(fi_ep_bind(*ep, &mr->fid, FI_REMOTE_READ | FI_REMOTE_WRITE), mr_bind);
    FI_RC(fi_enable(*ep), ep_enable);

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_CREATE_ENDPOINT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_Choose_provider(struct fi_info *prov, struct fi_info **prov_use)
{
    struct fi_info *p = prov;
    int i = 0;
    *prov_use = prov;
    if (MPIR_CVAR_OFI_DUMP_PROVIDERS) {
        fprintf(stderr, "Dumping Providers(first=%p):\n", prov);
        while (p) {
            fprintf(stdout, "%s", fi_tostr(p, FI_TYPE_INFO));
            p = p->next;
        }
    }
    return i;
}

#endif /* NETMOD_AM_OFI_INIT_H_INCLUDED */
