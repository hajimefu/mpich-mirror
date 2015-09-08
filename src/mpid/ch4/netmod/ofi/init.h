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
#ifndef NETMOD_OFI_INIT_H_INCLUDED
#define NETMOD_OFI_INIT_H_INCLUDED

#include "impl.h"
#include "mpich_cvars.h"
#include "pmi.h"

static inline int MPIDI_Choose_provider(info_t * prov, info_t ** prov_use);
static inline int MPIDI_Create_endpoint(info_t * prov_use,
                                        fid_domain_t domain,
                                        fid_cq_t p2p_cq,
                                        fid_cntr_t rma_ctr,
                                        fid_mr_t mr, fid_av_t av, fid_base_ep_t * ep, int index);
#define CHOOSE_PROVIDER(prov, prov_use,errstr)                          \
    do {                                                                \
    info_t *p = prov;                                                   \
    MPIR_ERR_CHKANDJUMP4(p==NULL, mpi_errno,MPI_ERR_OTHER,"**ofid_addrinfo", \
                         "**ofid_addrinfo %s %d %s %s",__SHORT_FILE__,  \
                         __LINE__,FCNAME, errstr);                      \
    MPIDI_Choose_provider(prov,prov_use);                               \
  } while (0);

#undef FUNCNAME
#define FUNCNAME MPIDI_netmod_init
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_netmod_init(int         rank,
                                    int         size,
                                    int         appnum,
                                    int        *tag_ub,
                                    MPID_Comm  *comm_world,
                                    MPID_Comm  *comm_self,
                                    int         spawned,
                                    int         num_contexts,
                                    void      **netmod_contexts)
{
    int mpi_errno = MPI_SUCCESS, pmi_errno, i, fi_version;
    int thr_err=0, str_errno, maxlen, iov_len;
    char *table = NULL, *provname = NULL;
    info_t *hints, *prov, *prov_use;
    uint64_t mr_flags;
    cq_attr_t cq_attr;
    cntr_attr_t cntr_attr;
    fi_addr_t *mapped_table;
    av_attr_t av_attr;
    char valS[MPIDI_KVSAPPSTRLEN], *val;
    char keyS[MPIDI_KVSAPPSTRLEN];
    size_t optlen;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_INIT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_INIT);

    CH4_COMPILE_TIME_ASSERT(offsetof(struct MPID_Request, dev.ch4.netmod) ==
                            offsetof(MPIDI_Chunk_request,context));
    CH4_COMPILE_TIME_ASSERT(offsetof(struct MPID_Request, dev.ch4.netmod) ==
                            offsetof(MPIDI_Huge_chunk_t,context));
    CH4_COMPILE_TIME_ASSERT(offsetof(struct MPID_Request, dev.ch4.netmod) ==
                            offsetof(MPIDI_Ctrl_req,context));
    CH4_COMPILE_TIME_ASSERT(offsetof(struct MPID_Request, dev.ch4.netmod) ==
                            offsetof(MPIDI_Ssendack_request,context));
    CH4_COMPILE_TIME_ASSERT(offsetof(struct MPID_Request, dev.ch4.netmod) ==
                            offsetof(MPIDI_Dynproc_req,context));
    CH4_COMPILE_TIME_ASSERT(offsetof(struct MPID_Request, dev.ch4.netmod) ==
                            offsetof(MPIDI_Win_request,context));
    CH4_COMPILE_TIME_ASSERT(sizeof(MPIDI_Devreq_t)>=sizeof(MPIDI_netmod_ofi_request_t));
    CH4_COMPILE_TIME_ASSERT(sizeof(MPID_Request)>=sizeof(MPIDI_Win_request));
    CH4_COMPILE_TIME_ASSERT(sizeof(MPIDI_Devgpid_t)>=sizeof(MPIDI_OFIGpid_t));

    *tag_ub = (1 << MPID_TAG_SHIFT) - 1;

    MPID_Thread_mutex_create(&MPIDI_THREAD_UTIL_MUTEX, &thr_err);
    MPID_Thread_mutex_create(&MPIDI_THREAD_PROGRESS_MUTEX, &thr_err);
    MPID_Thread_mutex_create(&MPIDI_THREAD_FI_MUTEX, &thr_err);
    MPID_Thread_mutex_create(&MPIDI_THREAD_SPAWN_MUTEX, &thr_err);

    /* ------------------------------------------------------------------------ */
    /* Hints to filter providers                                                */
    /* See man fi_getinfo for a list                                            */
    /* of all filters                                                           */
    /* mode:  Select capabilities ADI is prepared to support.                   */
    /*        In this case, ADI will pass in context into                       */
    /*        communication calls.                                              */
    /*        Note that we do not fill in FI_LOCAL_MR, which means this ADI     */
    /*        does not support exchange of memory regions on communication      */
    /*        calls. OFI requires that all communication calls use a registered */
    /*        mr but in our case this ADI is written to only support            */
    /*        transfers on a dynamic memory region that spans all of memory.    */
    /*        So, we do not set the FI_LOCAL_MR mode bit, and we set the        */
    /*        FI_SCALABLE_MR on the domain to  tell OFI our requirement and     */
    /*        filter providers appropriately                                    */
    /* caps:     Capabilities required from the provider.  The bits specified   */
    /*           with buffered receive, cancel, and remote complete implements  */
    /*           MPI semantics.                                                 */
    /*           Tagged: used to support tag matching, 2-sided                  */
    /*           RMA|Atomics:  supports MPI 1-sided                             */
    /*           MSG|MULTI_RECV:  Supports synchronization protocol for 1-sided */
    /*           We expect to register all memory up front for use with this    */
    /*           endpoint, so the ADI requires dynamic memory regions           */
    /* ------------------------------------------------------------------------ */

    /* ------------------------------------------------------------------------ */
    /* fi_allocinfo: allocate and zero an fi_info structure and all related     */
    /* substructures                                                            */
    /* ------------------------------------------------------------------------ */
    hints = fi_allocinfo();
    MPIU_Assert(hints != NULL);

    hints->mode = FI_CONTEXT;   /* We can handle contexts  */
    hints->caps = FI_TAGGED;    /* Tag matching interface  */
    hints->caps |= FI_MSG;      /* Message Queue apis      */
    hints->caps |= FI_MULTI_RECV;       /* Shared receive buffer   */
    hints->caps |= FI_RMA;      /* RMA(read/write)         */
    hints->caps |= FI_ATOMICS;  /* Atomics capabilities    */

    /* ------------------------------------------------------------------------ */
    /* FI_VERSION provides binary backward and forward compatibility support    */
    /* Specify the version of OFI is coded to, the provider will select struct  */
    /* layouts that are compatible with this version.                           */
    /* ------------------------------------------------------------------------ */
    fi_version = FI_VERSION(MPIDI_FI_MAJOR_VERSION, MPIDI_FI_MINOR_VERSION);

    /* ------------------------------------------------------------------------ */
    /* Set object options to be filtered by getinfo                             */
    /* domain_attr:  domain attribute requirements                              */
    /* op_flags:     persistent flag settings for an endpoint                   */
    /* endpoint type:  see FI_EP_RDM                                            */
    /* Filters applied (for this ADI, we need providers that can support):      */
    /* THREAD_ENDPOINT:  Progress serialization is handled by ADI (locking)     */
    /* PROGRESS_AUTO:  request providers that make progress without requiring   */
    /*                 the ADI to dedicate a thread to advance the state        */
    /* FI_DELIVERY_COMPLETE:  RMA operations are visible in remote memory       */
    /* FI_COMPLETION:  Selective completions of RMA ops                         */
    /* FI_EP_RDM:  Reliable datagram                                            */
    /* ------------------------------------------------------------------------ */
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

    /* ------------------------------------------------------------------------ */
    /* fi_getinfo:  returns information about fabric  services for reaching a   */
    /* remote node or service.  this does not necessarily allocate resources.   */
    /* Pass NULL for name/service because we want a list of providers supported */
    /* ------------------------------------------------------------------------ */
    provname = MPIR_CVAR_OFI_USE_PROVIDER ? (char *) MPIU_Strdup(MPIR_CVAR_OFI_USE_PROVIDER) : NULL;
    hints->fabric_attr->prov_name = provname;
    FI_RC(fi_getinfo(fi_version, NULL, NULL, 0ULL, hints, &prov), addrinfo);
    CHOOSE_PROVIDER(prov, &prov_use, "No suitable provider provider found");

    /* ------------------------------------------------------------------------ */
    /* Set global attributes attributes based on the provider choice            */
    /* ------------------------------------------------------------------------ */
    MPIDI_Global.max_buffered_send = prov_use->tx_attr->inject_size;
    MPIDI_Global.max_buffered_write = prov_use->tx_attr->inject_size;
    MPIDI_Global.max_send = prov_use->ep_attr->max_msg_size;
    MPIDI_Global.max_write = prov_use->ep_attr->max_msg_size;

    /* ------------------------------------------------------------------------ */
    /* Open fabric                                                              */
    /* The getinfo struct returns a fabric attribute struct that can be used to */
    /* instantiate the virtual or physical network.  This opens a "fabric       */
    /* provider".   We choose the first available fabric, but getinfo           */
    /* returns a list.                                                          */
    /* ------------------------------------------------------------------------ */
    FI_RC(fi_fabric(prov_use->fabric_attr, &MPIDI_Global.fabric, NULL), fabric);

    /* ------------------------------------------------------------------------ */
    /* Create the access domain, which is the physical or virtual network or    */
    /* hardware port/collection of ports.  Returns a domain object that can be  */
    /* used to create endpoints.                                                */
    /* ------------------------------------------------------------------------ */
    FI_RC(fi_domain(MPIDI_Global.fabric, prov_use, &MPIDI_Global.domain, NULL), opendomain);

    /* ------------------------------------------------------------------------ */
    /* Create the objects that will be bound to the endpoint.                   */
    /* The objects include:                                                     */
    /*     * dynamic memory-spanning memory region                              */
    /*     * completion queues for events                                       */
    /*     * counters for rma operations                                        */
    /*     * address vector of other endpoint addresses                         */
    /* ------------------------------------------------------------------------ */
    /* ------------------------------------------------------------------------ */
    /* Construct:  Memory region                                                */
    /* ------------------------------------------------------------------------ */
#ifdef MPIDI_USE_MR_OFFSET
    mr_flags = FI_MR_OFFSET;
#else
    mr_flags = 0ULL;
#endif
    FI_RC(fi_mr_reg(MPIDI_Global.domain,        /* In:  Domain Object       */
                    0,  /* In:  Lower memory address */
                    UINTPTR_MAX,        /* In:  Upper memory address */
                    FI_REMOTE_READ | FI_REMOTE_WRITE |  /* In:  Expose MR for read/write */
                    FI_SEND | FI_RECV, 0ULL,    /* In:  base MR offset      */
                    0ULL,       /* In:  requested key       */
                    mr_flags,   /* In:  flags               */
                    &MPIDI_Global.mr,   /* Out: memregion object    */
                    NULL), mr_reg);     /* In:  context             */
    MPIDI_Global.lkey = fi_mr_key(MPIDI_Global.mr);
    MPIU_Assert(MPIDI_Global.lkey == 0);

    /* ------------------------------------------------------------------------ */
    /* Construct:  Completion Queues                                            */
    /* ------------------------------------------------------------------------ */
    memset(&cq_attr, 0, sizeof(cq_attr));
    cq_attr.format = FI_CQ_FORMAT_TAGGED;
    FI_RC(fi_cq_open(MPIDI_Global.domain,       /* In:  Domain Object                */
                     &cq_attr,  /* In:  Configuration object         */
                     &MPIDI_Global.p2p_cq,      /* Out: CQ Object                    */
                     NULL), opencq);    /* In:  Context for cq events        */

    /* ------------------------------------------------------------------------ */
    /* Construct:  Counters                                                     */
    /* ------------------------------------------------------------------------ */
    memset(&cntr_attr, 0, sizeof(cntr_attr));
    cntr_attr.events = FI_CNTR_EVENTS_COMP;
    FI_RC(fi_cntr_open(MPIDI_Global.domain,     /* In:  Domain Object        */
                       &cntr_attr,      /* In:  Configuration object */
                       &MPIDI_Global.rma_ctr,   /* Out: Counter Object       */
                       NULL), openct);  /* Context: counter events   */

    /* ------------------------------------------------------------------------ */
    /* Construct:  Address Vector                                               */
    /* ------------------------------------------------------------------------ */
    memset(&av_attr, 0, sizeof(av_attr));
#ifdef MPIDI_USE_AV_TABLE
    av_attr.type = FI_AV_TABLE;
    av_attr.rx_ctx_bits = MPIDI_MAX_ENDPOINTS_BITS;
    MPIDI_Addr_table = (MPIDI_Addr_table_t *) MPIU_Malloc(sizeof(MPIDI_Addr_table_t));
    MPIDI_Addr_table->size = size;
    mapped_table = NULL;
#else
    av_attr.type = FI_AV_MAP;
    av_attr.rx_ctx_bits = MPIDI_MAX_ENDPOINTS_BITS;
    MPIDI_Addr_table =
        (MPIDI_Addr_table_t *) MPIU_Malloc(size * sizeof(fi_addr_t) + sizeof(MPIDI_Addr_table_t));
    MPIDI_Addr_table->size = size;
    mapped_table = (fi_addr_t *) MPIDI_Addr_table->table;
#endif
    FI_RC(fi_av_open(MPIDI_Global.domain,       /* In:  Domain Object         */
                     &av_attr,  /* In:  Configuration object  */
                     &MPIDI_Global.av,  /* Out: AV Object             */
                     NULL), avopen);    /* Context: AV events         */

    /* ------------------------------------------------------------------------ */
    /* Create a transport level communication endpoint.  To use the endpoint,   */
    /* it must be bound to completion counters or event queues and enabled,     */
    /* and the resources consumed by it, such as address vectors, counters,     */
    /* completion queues, etc.                                                  */
    /* ------------------------------------------------------------------------ */
    MPI_RC_POP(MPIDI_Create_endpoint(prov_use,
                                     MPIDI_Global.domain,
                                     MPIDI_Global.p2p_cq,
                                     MPIDI_Global.rma_ctr,
                                     MPIDI_Global.mr, MPIDI_Global.av, &MPIDI_Global.ep, 0));

    /* ---------------------------------- */
    /* Get our endpoint name and publish  */
    /* the socket to the KVS              */
    /* ---------------------------------- */
    MPIDI_Global.addrnamelen = FI_NAME_MAX;
    FI_RC(fi_getname((fid_t) MPIDI_Global.ep, MPIDI_Global.addrname,
                     &MPIDI_Global.addrnamelen), getname);
    MPIU_Assert(MPIDI_Global.addrnamelen <= FI_NAME_MAX);

    val = valS;
    str_errno = MPIU_STR_SUCCESS;
    maxlen = MPIDI_KVSAPPSTRLEN;
    memset(val, 0, maxlen);
    MPIU_STR_RC(MPIU_Str_add_binary_arg(&val, &maxlen, "OFI", (char *) &MPIDI_Global.addrname,
                                        MPIDI_Global.addrnamelen), buscard_len);
    PMI_RC(PMI_KVS_Get_my_name(MPIDI_Global.kvsname, MPIDI_KVSAPPSTRLEN), pmi);

    val = valS;
    sprintf(keyS, "OFI-%d", rank);
    PMI_RC(PMI_KVS_Put(MPIDI_Global.kvsname, keyS, val), pmi);
    PMI_RC(PMI_KVS_Commit(MPIDI_Global.kvsname), pmi);
    PMI_RC(PMI_Barrier(), pmi);

    /* -------------------------------- */
    /* Create our address table from    */
    /* encoded KVS values               */
    /* -------------------------------- */
    table = (char *) MPIU_Malloc(size * MPIDI_Global.addrnamelen);
    maxlen = MPIDI_KVSAPPSTRLEN;

    for (i = 0; i < size; i++) {
        sprintf(keyS, "OFI-%d", i);
        PMI_RC(PMI_KVS_Get(MPIDI_Global.kvsname, keyS, valS, MPIDI_KVSAPPSTRLEN), pmi);
        MPIU_STR_RC(MPIU_Str_get_binary_arg
                    (valS, "OFI", (char *) &table[i * MPIDI_Global.addrnamelen],
                     MPIDI_Global.addrnamelen, &maxlen), buscard_len);
    }



    /* -------------------------------- */
    /* Table is constructed.  Map it    */
    /* -------------------------------- */
    FI_RC(fi_av_insert(MPIDI_Global.av, table, size, mapped_table, 0ULL, NULL), avmap);

    /* -------------------------------- */
    /* Create the id to object maps     */
    /* -------------------------------- */
    MPIDI_OFI_Map_create(&MPIDI_Global.win_map);
    MPIDI_OFI_Map_create(&MPIDI_Global.comm_map);

    /* ------------------------------------------- */
    /* Post Buffers for Protocol Control Messages  */
    /* ------------------------------------------- */
    MPIDI_Global.num_ctrlblock = 2;
    iov_len = 128 * 1024 * MPID_MIN_CTRL_MSG_SZ;
    MPIDI_Global.iov = (iovec_t *) MPIU_Malloc(sizeof(iovec_t) * MPIDI_Global.num_ctrlblock);
    MPIDI_Global.msg = (msg_t *) MPIU_Malloc(sizeof(msg_t) * MPIDI_Global.num_ctrlblock);
    MPIDI_Global.control_req =
        (MPIDI_Ctrl_req *) MPIU_Malloc(sizeof(MPIDI_Ctrl_req) * MPIDI_Global.num_ctrlblock);
    MPIU_Assert(MPIDI_Global.iov != NULL);
    MPIU_Assert(MPIDI_Global.msg != NULL);
    optlen = MPID_MIN_CTRL_MSG_SZ;
    FI_RC(fi_setopt((fid_t) G_RXC_MSG(0),
                    FI_OPT_ENDPOINT, FI_OPT_MIN_MULTI_RECV, &optlen, sizeof(optlen)), setopt);

    for (i = 0; i < MPIDI_Global.num_ctrlblock; i++) {
        MPIDI_Global.iov[i].iov_base = MPIU_Malloc(iov_len);
        MPIU_Assert(MPIDI_Global.iov[i].iov_base != NULL);
        MPIDI_Global.iov[i].iov_len = iov_len;
        MPIDI_Global.msg[i].msg_iov = &MPIDI_Global.iov[i];
        MPIDI_Global.msg[i].desc = (void **) &MPIDI_Global.mr;
        MPIDI_Global.msg[i].iov_count = 1;
        MPIDI_Global.msg[i].addr = FI_ADDR_UNSPEC;
        MPIDI_Global.msg[i].context = &MPIDI_Global.control_req[i].context;
        MPIDI_Global.control_req[i].event_id = MPIDI_EVENT_CONTROL;
        MPIDI_Global.msg[i].data = 0;
        FI_RC_RETRY(fi_recvmsg(G_RXC_MSG(0), &MPIDI_Global.msg[i],
                               FI_MULTI_RECV | FI_COMPLETION), prepost);
    }

    /* ---------------------------------- */
    /* Initialize MPI_COMM_SELF and VCRT  */
    /* ---------------------------------- */
    MPI_RC_POP(MPIDI_OFI_VCRT_Create(comm_self->remote_size, &COMM_OFI(comm_self).vcrt));
    COMM_OFI(comm_self).vcrt->vcr_table[0].addr_idx = rank;
    COMM_OFI(comm_self).vcrt->vcr_table[0].is_local = 1;

    /* ---------------------------------- */
    /* Initialize MPI_COMM_WORLD and VCRT */
    /* ---------------------------------- */
    MPI_RC_POP(MPIDI_OFI_VCRT_Create(comm_world->remote_size, &COMM_OFI(comm_world).vcrt));

    /* -------------------------------- */
    /* Calculate per-node map           */
    /* -------------------------------- */
    MPIDI_Global.node_map = (MPID_Node_id_t *)
        MPIU_Malloc(comm_world->local_size*sizeof(*MPIDI_Global.node_map));
    MPIDI_CH4U_build_nodemap(comm_world->rank,
                             comm_world,
                             comm_world->local_size,
                             MPIDI_Global.node_map,
                             &MPIDI_Global.max_node_id);

    for(i=0; i<comm_world->local_size; i++)
        COMM_OFI(comm_world).vcrt->vcr_table[i].is_local =
            (MPIDI_Global.node_map[i] == MPIDI_Global.node_map[comm_world->rank])?1:0;

    MPIR_Datatype_init_names();
    MPIDI_OFI_Index_datatypes();

    /* -------------------------------- */
    /* Initialize Dynamic Tasking       */
    /* -------------------------------- */
    if (spawned) {
        char parent_port[MPIDI_MAX_KVS_VALUE_LEN];
        PMI_RC(PMI_KVS_Get(MPIDI_Global.kvsname,
                           MPIDI_PARENT_PORT_KVSKEY, parent_port, MPIDI_MAX_KVS_VALUE_LEN), pmi);
        MPI_RC_POP(MPIDI_Comm_connect(parent_port, NULL, 0, comm_world, &MPIR_Process.comm_parent));
        MPIU_Assert(MPIR_Process.comm_parent != NULL);
        MPIU_Strncpy(MPIR_Process.comm_parent->name, "MPI_COMM_PARENT", MPI_MAX_OBJECT_NAME);
    }

  fn_exit:

    /* -------------------------------- */
    /* Free temporary resources         */
    /* -------------------------------- */
    if (provname) {
        MPIU_Free(provname);
        hints->fabric_attr->prov_name = NULL;
    }

    if (prov)
        fi_freeinfo(prov);

    fi_freeinfo(hints);

    if (table)
        MPIU_Free(table);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_INIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_netmod_finalize(void)
{
    int thr_err=0,mpi_errno = MPI_SUCCESS;
    int i = 0;
    int barrier[2] = { 0 };
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPID_Comm *comm;

    /* Barrier over allreduce, but force non-immediate send */
    MPIDI_Global.max_buffered_send = 0;
    MPI_RC_POP(MPIR_Allreduce_impl(&barrier[0], &barrier[1], 1, MPI_INT,
                                   MPI_SUM, MPIR_Process.comm_world, &errflag));

#ifdef MPIDI_USE_SCALABLE_ENDPOINTS

    for (i = 0; i < MPIDI_Global.num_ctrlblock; i++)
        FI_RC(fi_cancel((fid_t) G_RXC_MSG(0), &MPIDI_Global.control_req[i].context), ctrlcancel);

    FI_RC(fi_close((fid_t) G_TXC_TAG(0)), epclose);
    FI_RC(fi_close((fid_t) G_TXC_RMA(0)), epclose);
    FI_RC(fi_close((fid_t) G_TXC_MSG(0)), epclose);
    FI_RC(fi_close((fid_t) G_TXC_CTR(0)), epclose);

    FI_RC(fi_close((fid_t) G_RXC_TAG(0)), epclose);
    FI_RC(fi_close((fid_t) G_RXC_RMA(0)), epclose);
    FI_RC(fi_close((fid_t) G_RXC_MSG(0)), epclose);
    FI_RC(fi_close((fid_t) G_RXC_CTR(0)), epclose);
#else /* MPIDI_USE_SCALABLE_ENDPOINTS not defined */

    for (i = 0; i < MPIDI_Global.num_ctrlblock; i++)
        FI_RC(fi_cancel((fid_t) MPIDI_Global.ep, &MPIDI_Global.control_req[i].context), ctrlcancel);

#endif
    FI_RC(fi_close((fid_t) MPIDI_Global.mr), mr_unreg);
    FI_RC(fi_close((fid_t) MPIDI_Global.ep), epclose);
    FI_RC(fi_close((fid_t) MPIDI_Global.av), avclose);
    FI_RC(fi_close((fid_t) MPIDI_Global.p2p_cq), cqclose);
    FI_RC(fi_close((fid_t) MPIDI_Global.rma_ctr), cqclose);
    FI_RC(fi_close((fid_t) MPIDI_Global.domain), domainclose);

    /* --------------------------------------- */
    /* Free comm world VCRT and addr table     */
    /* comm_release will also releace the vcrt */
    /* --------------------------------------- */
    comm = MPIR_Process.comm_world;
    MPIR_Comm_release_always(comm);

    comm = MPIR_Process.comm_self;
    MPIR_Comm_release_always(comm);

    MPIU_Free(MPIDI_Addr_table);

    MPIU_Free(MPIDI_Global.node_map);

    for (i = 0; i < MPIDI_Global.num_ctrlblock; i++)
        MPIU_Free(MPIDI_Global.iov[i].iov_base);

    MPIU_Free(MPIDI_Global.iov);
    MPIU_Free(MPIDI_Global.msg);
    MPIU_Free(MPIDI_Global.control_req);

    MPIDI_OFI_Map_destroy(MPIDI_Global.win_map);
    MPIDI_OFI_Map_destroy(MPIDI_Global.comm_map);

    PMI_Finalize();

    MPID_Thread_mutex_destroy(&MPIDI_THREAD_UTIL_MUTEX, &thr_err);
    MPID_Thread_mutex_destroy(&MPIDI_THREAD_PROGRESS_MUTEX, &thr_err);
    MPID_Thread_mutex_destroy(&MPIDI_THREAD_FI_MUTEX, &thr_err);
    MPID_Thread_mutex_destroy(&MPIDI_THREAD_SPAWN_MUTEX, &thr_err);

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline void *MPIDI_netmod_alloc_mem(size_t size, MPID_Info * info_ptr)
{

    void *ap;
    ap = MPIU_Malloc(size);
    return ap;
}

static inline int MPIDI_netmod_free_mem(void *ptr)
{
    int mpi_errno = MPI_SUCCESS;
    MPIU_Free(ptr);

    return mpi_errno;
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
    MPIDI_OFI_VCRT_Create(size, &COMM_OFI(newcomm_ptr).vcrt);

    for (i = 0; i < size; i++)
    {
        COMM_OFI(newcomm_ptr).vcrt->vcr_table[i].addr_idx = lpids[i];
        COMM_OFI(newcomm_ptr).vcrt->vcr_table[i].is_local = 0;
    }
    return 0;
}


#undef FUNCNAME
#define FUNCNAME MPIDI_Create_endpoint
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_Create_endpoint(info_t * prov_use,
                                        fid_domain_t domain,
                                        fid_cq_t p2p_cq,
                                        fid_cntr_t rma_ctr,
                                        fid_mr_t mr, fid_av_t av, fid_base_ep_t * ep, int index)
{
    int mpi_errno = MPI_SUCCESS;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_CREATE_ENDPOINT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_CREATE_ENDPOINT);

#ifdef MPIDI_USE_SCALABLE_ENDPOINTS
    FI_RC(fi_scalable_ep(domain, prov_use, ep, NULL), ep);
    FI_RC(fi_scalable_ep_bind(*ep, (fid_t) av, 0), bind);

    tx_attr_t tx_attr;
    tx_attr = *prov_use->tx_attr;
    tx_attr.caps = FI_TAGGED;
    tx_attr.caps |= FI_DELIVERY_COMPLETE;
    tx_attr.op_flags = FI_DELIVERY_COMPLETE;
    FI_RC(fi_tx_context(*ep, index, &tx_attr, &G_TXC_TAG(index), NULL), ep);
    FI_RC(fi_ep_bind(G_TXC_TAG(index), (fid_t) p2p_cq, FI_SEND), bind);

    tx_attr = *prov_use->tx_attr;
    tx_attr.caps = FI_RMA;
    tx_attr.caps |= FI_ATOMICS;
    tx_attr.caps |= FI_DELIVERY_COMPLETE;
    tx_attr.op_flags = FI_DELIVERY_COMPLETE;
    FI_RC(fi_tx_context(*ep, index + 1, &tx_attr, &G_TXC_RMA(index), NULL), ep);
    FI_RC(fi_ep_bind(G_TXC_RMA(index), (fid_t) p2p_cq, FI_SEND), bind);

    tx_attr = *prov_use->tx_attr;
    tx_attr.caps = FI_MSG;
    tx_attr.op_flags = 0;
    FI_RC(fi_tx_context(*ep, index + 2, &tx_attr, &G_TXC_MSG(index), NULL), ep);
    FI_RC(fi_ep_bind(G_TXC_MSG(index), (fid_t) p2p_cq, FI_SEND), bind);

    tx_attr = *prov_use->tx_attr;
    tx_attr.caps = FI_RMA;
    tx_attr.caps |= FI_ATOMICS;
    tx_attr.caps |= FI_DELIVERY_COMPLETE;
    tx_attr.op_flags = FI_DELIVERY_COMPLETE;
    FI_RC(fi_tx_context(*ep, index + 3, &tx_attr, &G_TXC_CTR(index), NULL), ep);
    FI_RC(fi_ep_bind(G_TXC_CTR(index), (fid_t) rma_ctr, FI_WRITE | FI_READ), bind);

    rx_attr_t rx_attr;
    rx_attr = *prov_use->rx_attr;
    rx_attr.caps = FI_TAGGED;
    rx_attr.caps |= FI_DELIVERY_COMPLETE;
    rx_attr.op_flags = 0;
    FI_RC(fi_rx_context(*ep, index, &rx_attr, &G_RXC_TAG(index), NULL), ep);

    FI_RC(fi_ep_bind(G_RXC_TAG(index), (fid_t) p2p_cq, FI_RECV), bind);

    rx_attr = *prov_use->rx_attr;
    rx_attr.caps = FI_RMA;
    rx_attr.caps |= FI_ATOMICS;
    rx_attr.op_flags = 0;
    FI_RC(fi_rx_context(*ep, index + 1, &rx_attr, &G_RXC_RMA(index), NULL), ep);

    rx_attr = *prov_use->rx_attr;
    rx_attr.caps = FI_MSG;
    rx_attr.caps |= FI_MULTI_RECV;
    rx_attr.op_flags = FI_MULTI_RECV;
    FI_RC(fi_rx_context(*ep, index + 2, &rx_attr, &G_RXC_MSG(index), NULL), ep);
    FI_RC(fi_ep_bind(G_RXC_MSG(index), (fid_t) p2p_cq, FI_RECV), bind);

    rx_attr = *prov_use->rx_attr;
    rx_attr.caps = FI_RMA;
    rx_attr.caps |= FI_ATOMICS;
    rx_attr.op_flags = 0;
    FI_RC(fi_rx_context(*ep, index + 3, &rx_attr, &G_RXC_CTR(index), NULL), ep);

    FI_RC(fi_enable(G_TXC_TAG(index)), ep_enable);
    FI_RC(fi_enable(G_TXC_RMA(index)), ep_enable);
    FI_RC(fi_enable(G_TXC_MSG(index)), ep_enable);
    FI_RC(fi_enable(G_TXC_CTR(index)), ep_enable);

    FI_RC(fi_enable(G_RXC_TAG(index)), ep_enable);
    FI_RC(fi_enable(G_RXC_RMA(index)), ep_enable);
    FI_RC(fi_enable(G_RXC_MSG(index)), ep_enable);
    FI_RC(fi_enable(G_RXC_CTR(index)), ep_enable);

    FI_RC(fi_ep_bind(G_TXC_RMA(0), (fid_t) mr, FI_REMOTE_READ | FI_REMOTE_WRITE), bind);
    FI_RC(fi_ep_bind(G_RXC_RMA(0), (fid_t) mr, FI_REMOTE_READ | FI_REMOTE_WRITE), bind);
    FI_RC(fi_ep_bind(G_TXC_CTR(0), (fid_t) mr, FI_REMOTE_READ | FI_REMOTE_WRITE), bind);
    FI_RC(fi_ep_bind(G_RXC_CTR(0), (fid_t) mr, FI_REMOTE_READ | FI_REMOTE_WRITE), bind);
#else
    /* ---------------------------------------------------------- */
    /* Bind the MR, CQs, counters,  and AV to the endpoint object */
    /* ---------------------------------------------------------- */
    FI_RC(fi_endpoint(domain, prov_use, ep, NULL), ep);
    FI_RC(fi_ep_bind(*ep, (fid_t) p2p_cq, FI_SEND | FI_RECV | FI_SELECTIVE_COMPLETION), bind);
    FI_RC(fi_ep_bind(*ep, (fid_t) rma_ctr, FI_READ | FI_WRITE), bind);
    FI_RC(fi_ep_bind(*ep, (fid_t) av, 0), bind);
    FI_RC(fi_enable(*ep), ep_enable);
    FI_RC(fi_ep_bind(*ep, (fid_t) mr, FI_REMOTE_READ | FI_REMOTE_WRITE), bind);
#endif /* MPIDI_USE_SCALABLE_ENDPOINTS */

  fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_CREATE_ENDPOINT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_Choose_provider(info_t * prov, info_t ** prov_use)
{
    info_t *p = prov;
    int i = 0;
    *prov_use = prov;

    if (MPIR_CVAR_OFI_DUMP_PROVIDERS) {
        fprintf(stdout, "Dumping Providers(first=%p):\n", prov);

        while (p) {
            fprintf(stdout, "%s", fi_tostr(p, FI_TYPE_INFO));
            p = p->next;
        }
    }

    return i;
}

#endif /* NETMOD_OFI_INIT_H_INCLUDED */
