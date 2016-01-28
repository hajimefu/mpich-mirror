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
#ifndef NETMOD_OFI_DYNPROC_H_INCLUDED
#define NETMOD_OFI_DYNPROC_H_INCLUDED

#include "impl.h"

#define MPIDI_CH4_NMI_OFI_PORT_NAME_TAG_KEY "tag"
#define MPIDI_CH4_NMI_OFI_CONNENTRY_TAG_KEY "connentry"

#ifdef MPIDI_CH4_NMI_OFI_CONFIG_USE_AV_TABLE
#define MPIDI_CH4_NMI_OFI_TABLE_INDEX_INCR() MPIDI_Addr_table->size++
#else
#define MPIDI_CH4_NMI_OFI_TABLE_INDEX_INCR()
#endif

static inline void MPIDI_CH4_NMI_OFI_Free_port_name_tag(int tag)
{
    int index, rem_tag;

    index = tag / (sizeof(int) * 8);
    rem_tag = tag - (index * sizeof(int) * 8);

    MPIDI_Global.port_name_tag_mask[index] &= ~(1 << ((8 * sizeof(int)) - 1 - rem_tag));
}


static inline int MPIDI_CH4_NMI_OFI_Get_port_name_tag(int *port_name_tag)
{
    unsigned i, j;
    int mpi_errno = MPI_SUCCESS;

    for(i = 0; i < MPIR_MAX_CONTEXT_MASK; i++)
        if(MPIDI_Global.port_name_tag_mask[i] != ~0)
            break;

    if(i < MPIR_MAX_CONTEXT_MASK)
        for(j = 0; j < (8 * sizeof(int)); j++) {
            if((MPIDI_Global.port_name_tag_mask[i] | (1 << ((8 * sizeof(int)) - j - 1))) !=
               MPIDI_Global.port_name_tag_mask[i]) {
                MPIDI_Global.port_name_tag_mask[i] |= (1 << ((8 * sizeof(int)) - j - 1));
                *port_name_tag = ((i * 8 * sizeof(int)) + j);
                goto fn_exit;
            }
        }
    else
        goto fn_fail;

fn_exit:
    return mpi_errno;

fn_fail:
    *port_name_tag = -1;
    mpi_errno = MPI_ERR_OTHER;
    goto fn_exit;
}

static inline int MPIDI_CH4_NMI_OFI_Get_tag_from_port(const char *port_name,
                                                      int        *port_name_tag)
{
    int mpi_errno = MPI_SUCCESS;
    int str_errno = MPL_STR_SUCCESS;

    if(strlen(port_name) == 0)
        goto fn_exit;

    str_errno = MPL_str_get_int_arg(port_name,
                                     MPIDI_CH4_NMI_OFI_PORT_NAME_TAG_KEY,
                                     port_name_tag);
    MPIR_ERR_CHKANDJUMP(str_errno, mpi_errno, MPI_ERR_OTHER,
                        "**argstr_no_port_name_tag");
fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


static inline int MPIDI_CH4_NMI_OFI_Get_conn_name_from_port(const char *port_name,
                                                            char       *connname)
{
    int mpi_errno = MPI_SUCCESS;
    int maxlen    = MPIDI_KVSAPPSTRLEN;
    MPL_str_get_binary_arg(port_name,
                            MPIDI_CH4_NMI_OFI_CONNENTRY_TAG_KEY,
                            connname,
                            MPIDI_Global.addrnamelen,
                            &maxlen);
    return mpi_errno;
}

static inline int MPIDI_CH4_NMI_OFI_Dynproc_create_intercomm(const char      *port_name,
                                                             char            *addr_table,
                                                             MPID_Node_id_t  *node_table,
                                                             int              entries,
                                                             MPID_Comm       *comm_ptr,
                                                             MPID_Comm      **newcomm,
                                                             int              is_low_group,
                                                             char            *api)
{
    int        start,i,context_id_offset,mpi_errno = MPI_SUCCESS;
    MPID_Comm *tmp_comm_ptr = NULL;
    fi_addr_t *addr = NULL;

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Get_tag_from_port(port_name,&context_id_offset));
    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIR_Comm_create(&tmp_comm_ptr));

    tmp_comm_ptr->context_id     = MPID_CONTEXT_SET_FIELD(DYNAMIC_PROC,
                                                          context_id_offset,
                                                          1);
    tmp_comm_ptr->recvcontext_id       = tmp_comm_ptr->context_id;
    tmp_comm_ptr->remote_size          = entries;
    tmp_comm_ptr->local_size           = comm_ptr->local_size;
    tmp_comm_ptr->rank                 = comm_ptr->rank;
    tmp_comm_ptr->comm_kind            = MPID_INTERCOMM;
    tmp_comm_ptr->local_comm           = comm_ptr;
    tmp_comm_ptr->is_low_group         = is_low_group;
    MPIDI_CH4_NMI_OFI_COMM(tmp_comm_ptr).local_vcrt = MPIDI_CH4_NMI_OFI_COMM(comm_ptr).vcrt;

    mpi_errno = MPIDI_CH4_NMI_OFI_VCRT_Create(tmp_comm_ptr->remote_size,
                                              &MPIDI_CH4_NMI_OFI_COMM(tmp_comm_ptr).vcrt);
    start = MPIDI_Addr_table->size;
    MPIDI_Global.node_map = (MPID_Node_id_t *)MPL_realloc(MPIDI_Global.node_map,
                                                           (entries+start)*sizeof(MPID_Node_id_t));

    for(i=0; i<entries; i++)
        MPIDI_Global.node_map[start+i] = node_table[i];

#ifndef MPIDI_CH4_NMI_OFI_CONFIG_USE_AV_TABLE
    MPIDI_Addr_table = (MPIDI_CH4_NMI_OFI_Addr_table_t *)MPL_realloc(MPIDI_Addr_table,
                                                                      (entries+start)*sizeof(fi_addr_t)+
                                                                      sizeof(MPIDI_CH4_NMI_OFI_Addr_table_t));
    addr=&(MPIDI_CH4_NMI_OFI_TO_PHYS(start));
#endif

    MPIDI_Addr_table->size += entries;

    for(i=0; i<entries; i++) {
        MPIDI_CH4_NMI_OFI_COMM(tmp_comm_ptr).vcrt->vcr_table[i].addr_idx += start;
    }

    MPIDI_CH4_NMI_OFI_CALL(fi_av_insert(MPIDI_Global.av,addr_table,entries,
                                        addr,0ULL,NULL),avmap);
    /*    *newcomm = tmp_comm_ptr;  not needed to be intialized */
    MPIR_Comm_commit(tmp_comm_ptr);

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIR_Comm_dup_impl(tmp_comm_ptr, newcomm));

    /*    MPIDI_CH4_NMI_OFI_VCRT_Release(MPIDI_CH4_NMI_OFI_COMM(tmp_comm_ptr).vcrt);
        MPIU_Handle_obj_free(&MPID_Comm_mem, tmp_comm_ptr);  this is done in comm_release */
    tmp_comm_ptr->local_comm = NULL; /* avoid freeing local comm with comm_release */
    MPIR_Comm_release(tmp_comm_ptr);

    MPL_free(addr_table);
    MPL_free(node_table);

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

static inline int MPIDI_CH4_NMI_OFI_Dynproc_bcast(int              root,
                                                  MPID_Comm       *comm_ptr,
                                                  int             *out_root,
                                                  ssize_t         *out_table_size,
                                                  char           **out_addr_table,
                                                  MPID_Node_id_t **out_node_table)
{
    int            entries, mpi_errno =  MPI_SUCCESS;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIR_Bcast_intra(out_root,1,MPI_INT,
                                                    root,comm_ptr,&errflag));
    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIR_Bcast_intra(out_table_size, 1, MPI_LONG_LONG_INT,
                                                    root, comm_ptr, &errflag));

    if(*out_addr_table == NULL)
        *out_addr_table  = (char *)MPL_malloc(*out_table_size);

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIR_Bcast_intra(*out_addr_table, *out_table_size,
                                                    MPI_CHAR,root, comm_ptr, &errflag));

    entries     = *out_table_size/MPIDI_Global.addrnamelen;

    if(*out_node_table == NULL)
        *out_node_table  = (MPID_Node_id_t *)MPL_malloc(MPIDI_Global.addrnamelen*entries);

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIR_Bcast_intra(*out_node_table, entries*sizeof(MPID_Node_id_t),
                                                    MPI_CHAR,root, comm_ptr, &errflag));

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}


static inline int MPIDI_CH4_NMI_OFI_Dynproc_exchange_map(int              root,
                                                         int              phase,
                                                         int              port_id,
                                                         fi_addr_t       *conn,
                                                         char            *conname,
                                                         MPID_Comm       *comm_ptr,
                                                         ssize_t         *out_table_size,
                                                         int             *out_root,
                                                         char           **out_addr_table,
                                                         MPID_Node_id_t **out_node_table)
{
    int i,mpi_errno = MPI_SUCCESS;

    MPIDI_CH4_NMI_OFI_Dynamic_process_request_t req[2];
    uint64_t          match_bits    = 0;
    uint64_t          mask_bits     = 0;
    struct fi_msg_tagged      msg;
    req[0].done            = MPIDI_CH4_NMI_OFI_PEEK_START;
    req[0].event_id        = MPIDI_CH4_NMI_OFI_EVENT_ACCEPT_PROBE;
    req[1].done            = MPIDI_CH4_NMI_OFI_PEEK_START;
    req[1].event_id        = MPIDI_CH4_NMI_OFI_EVENT_ACCEPT_PROBE;
    match_bits             = MPIDI_CH4_NMI_OFI_Init_recvtag(&mask_bits,port_id,
                                                            MPI_ANY_SOURCE,
                                                            MPI_ANY_TAG);
    match_bits            |= MPIDI_CH4_NMI_OFI_DYNPROC_SEND;

    if(phase == 0) {
        /* Receive the addresses                           */
        /* We don't know the size, so probe for table size */
        /* Receive phase updates the connection            */
        /* With the probed address                         */
        msg.msg_iov   = NULL;
        msg.desc      = NULL;
        msg.iov_count = 0;
        msg.addr      = FI_ADDR_UNSPEC;
        msg.tag       = match_bits;
        msg.ignore    = mask_bits;
        msg.context   = (void *) &req[0].context;
        msg.data      = 0;

        while(req[0].done != MPIDI_CH4_NMI_OFI_PEEK_FOUND) {
            req[0].done = MPIDI_CH4_NMI_OFI_PEEK_START;
            MPIDI_CH4_NMI_OFI_CALL(fi_trecvmsg(MPIDI_CH4_NMI_OFI_EP_RX_TAG(0),&msg,FI_PEEK|FI_COMPLETION),trecv);
            MPIDI_CH4_NMI_OFI_PROGRESS_WHILE(req[0].done == MPIDI_CH4_NMI_OFI_PEEK_START);
        }

        *out_table_size    = req[0].msglen;
        *out_root          = req[0].tag;
        *out_addr_table    = (char *)MPL_malloc(*out_table_size);

        int entries        = req[0].msglen/MPIDI_Global.addrnamelen;
        *out_node_table    = (MPID_Node_id_t *)MPL_malloc(entries*sizeof(MPID_Node_id_t));

        req[0].done     = 0;
        req[0].event_id = MPIDI_CH4_NMI_OFI_EVENT_DYNPROC_DONE;
        req[1].done     = 0;
        req[1].event_id = MPIDI_CH4_NMI_OFI_EVENT_DYNPROC_DONE;

        MPIDI_CH4_NMI_OFI_CALL_RETRY(fi_trecv(MPIDI_CH4_NMI_OFI_EP_RX_TAG(0),
                                              *out_addr_table,
                                              *out_table_size,
                                              NULL,
                                              FI_ADDR_UNSPEC,
                                              match_bits,
                                              mask_bits,
                                              &req[0].context),trecv);
        MPIDI_CH4_NMI_OFI_CALL_RETRY(fi_trecv(MPIDI_CH4_NMI_OFI_EP_RX_TAG(0),
                                              *out_node_table,
                                              entries*sizeof(MPID_Node_id_t),
                                              NULL,
                                              FI_ADDR_UNSPEC,
                                              match_bits,
                                              mask_bits,
                                              &req[1].context),trecv);

        MPIDI_CH4_NMI_OFI_PROGRESS_WHILE(!req[0].done || !req[1].done);
        memcpy(conname, *out_addr_table+req[0].source*MPIDI_Global.addrnamelen, MPIDI_Global.addrnamelen);
    }

    if(phase == 1) {
        /* Send our table to the child */
        /* Send phase maps the entry   */
        char   *my_addr_table;
        int     tag   = root;
        int     tblsz = MPIDI_Global.addrnamelen*comm_ptr->local_size;
        my_addr_table       = (char *)MPL_malloc(tblsz);

        MPID_Node_id_t *my_node_table;
        MPID_Node_id_t  nodetblsz = sizeof(*my_node_table)*comm_ptr->local_size;
        my_node_table             = (MPID_Node_id_t *)MPL_malloc(nodetblsz);

        match_bits                = MPIDI_CH4_NMI_OFI_Init_sendtag(port_id,
                                                                   comm_ptr->rank,
                                                                   tag,MPIDI_CH4_NMI_OFI_DYNPROC_SEND);

        for(i=0; i<comm_ptr->local_size; i++) {
            size_t sz = MPIDI_Global.addrnamelen;
            MPIDI_CH4_NMI_OFI_CALL(fi_av_lookup(MPIDI_Global.av,
                                                MPIDI_CH4_NMI_OFI_COMM_TO_PHYS(comm_ptr, i),
                                                my_addr_table+i*MPIDI_Global.addrnamelen,
                                                &sz),avlookup);
            MPIU_Assert(sz == MPIDI_Global.addrnamelen);
        }

        for(i=0; i<comm_ptr->local_size; i++)
            my_node_table[i] = MPIDI_Global.node_map[MPIDI_CH4_NMI_OFI_COMM_TO_INDEX(comm_ptr,i)];

        /* fi_av_map here is not quite right for some providers */
        /* we need to get this connection from the sockname     */
        req[0].done     = 0;
        req[0].event_id = MPIDI_CH4_NMI_OFI_EVENT_DYNPROC_DONE;
        req[1].done     = 0;
        req[1].event_id = MPIDI_CH4_NMI_OFI_EVENT_DYNPROC_DONE;
        MPIDI_CH4_NMI_OFI_CALL_RETRY(fi_tsend(MPIDI_CH4_NMI_OFI_EP_TX_TAG(0),
                                              my_addr_table,
                                              tblsz,
                                              NULL,
                                              *conn,
                                              match_bits,
                                              (void *) &req[0].context),tsend);
        MPIDI_CH4_NMI_OFI_CALL_RETRY(fi_tsend(MPIDI_CH4_NMI_OFI_EP_TX_TAG(0),
                                              my_node_table,
                                              nodetblsz,
                                              NULL,
                                              *conn,
                                              match_bits,
                                              (void *) &req[1].context),tsend);

        MPIDI_CH4_NMI_OFI_PROGRESS_WHILE(!req[0].done || !req[1].done);

        MPL_free(my_addr_table);
        MPL_free(my_node_table);
    }

fn_exit:
    return mpi_errno;
fn_fail:
    goto fn_exit;
}



#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_comm_connect
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_comm_connect(const char *port_name,
                                            MPID_Info  *info,
                                            int         root,
                                            MPID_Comm  *comm_ptr,
                                            MPID_Comm **newcomm)
{
    int                entries,mpi_errno = MPI_SUCCESS;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_COMM_CONNECT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_COMM_CONNECT);

    char           *parent_addr_table = NULL;
    MPID_Node_id_t *parent_node_table = NULL;
    ssize_t         parent_table_sz   = -1LL;
    int             parent_root       = -1;
    int             rank              = comm_ptr->rank;
    int             port_id;

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_SPAWN_MUTEX);
    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Get_tag_from_port(port_name,&port_id));

    if(rank == root) {
        fi_addr_t conn;
        char      conname[FI_NAME_MAX];
        MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Get_conn_name_from_port(port_name,conname));
        MPIDI_CH4_NMI_OFI_CALL(fi_av_insert(MPIDI_Global.av,conname,1,&conn,0ULL,NULL),avmap);
        MPIDI_CH4_NMI_OFI_TABLE_INDEX_INCR();
        MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Dynproc_exchange_map(root,1,port_id,&conn,conname,comm_ptr,
                                                                              &parent_table_sz,&parent_root,
                                                                              &parent_addr_table,
                                                                              &parent_node_table));
        MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Dynproc_exchange_map(root,0,port_id,&conn,conname,comm_ptr,
                                                                              &parent_table_sz,&parent_root,
                                                                              &parent_addr_table,
                                                                              &parent_node_table));
        MPIDI_CH4_NMI_OFI_CALL(fi_av_remove(MPIDI_Global.av, &conn,1,0ULL),avmap);
    }

    /* Map the new address table */
    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Dynproc_bcast(root,comm_ptr,&parent_root,
                                                                   &parent_table_sz,
                                                                   &parent_addr_table,
                                                                   &parent_node_table));

    /* Now Create the New Intercomm */
    entries   = parent_table_sz/MPIDI_Global.addrnamelen;
    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Dynproc_create_intercomm(port_name,
                                                                              parent_addr_table,
                                                                              parent_node_table,
                                                                              entries,
                                                                              comm_ptr,
                                                                              newcomm,
                                                                              0,
                                                                              (char *)"Connect"));
fn_exit:
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_SPAWN_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_COMM_CONNECT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_comm_disconnect
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_comm_disconnect(MPID_Comm *comm_ptr)
{
    int            mpi_errno = MPI_SUCCESS;
    MPIR_Errflag_t errflag   = MPIR_ERR_NONE;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_COMM_DISCONNECT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_COMM_DISCONNECT);

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIR_Barrier_impl(comm_ptr, &errflag));
    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIR_Comm_free_impl(comm_ptr));

fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_COMM_DISCONNECT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_comm_open_port
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_open_port(MPID_Info *info_ptr, char *port_name)
{
    int mpi_errno     = MPI_SUCCESS;
    int str_errno     = MPL_STR_SUCCESS;
    int port_name_tag = 0;
    int len           = MPI_MAX_PORT_NAME;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_COMM_OPEN_PORT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_COMM_OPEN_PORT);

    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Get_port_name_tag(&port_name_tag));
    MPIDI_CH4_NMI_OFI_STR_CALL(MPL_str_add_int_arg(&port_name,&len,MPIDI_CH4_NMI_OFI_PORT_NAME_TAG_KEY,
                                                    port_name_tag),port_str);
    MPIDI_CH4_NMI_OFI_STR_CALL(MPL_str_add_binary_arg(&port_name,&len,MPIDI_CH4_NMI_OFI_CONNENTRY_TAG_KEY,
                                                       MPIDI_Global.addrname,
                                                       MPIDI_Global.addrnamelen),port_str);
fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_COMM_OPEN_PORT);
    return mpi_errno;
fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_comm_close_port
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_close_port(const char *port_name)
{
    int mpi_errno = MPI_SUCCESS;
    int port_name_tag;

    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_COMM_CLOSE_PORT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_COMM_CLOSE_PORT);

    mpi_errno = MPIDI_CH4_NMI_OFI_Get_tag_from_port(port_name, &port_name_tag);
    MPIDI_CH4_NMI_OFI_Free_port_name_tag(port_name_tag);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_COMM_CLOSE_PORT);
    return mpi_errno;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_CH4_NM_comm_close_port
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
static inline int MPIDI_CH4_NM_comm_accept(const char *port_name,
                                           MPID_Info  *info,
                                           int         root,
                                           MPID_Comm  *comm_ptr,
                                           MPID_Comm **newcomm)
{
    int             entries,mpi_errno = MPI_SUCCESS;
    char           *child_addr_table    = NULL;
    MPID_Node_id_t *child_node_table    = NULL;
    ssize_t         child_table_sz      = -1LL;
    int             child_root          = -1;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_COMM_CLOSE_PORT);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_COMM_CLOSE_PORT);

    MPID_THREAD_CS_ENTER(POBJ,MPIDI_CH4_NMI_OFI_THREAD_SPAWN_MUTEX);
    int rank = comm_ptr->rank;

    if(rank == root) {
        fi_addr_t conn;
        char      conname[FI_NAME_MAX];
        int       port_id;
        MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Get_tag_from_port(port_name,&port_id));
        MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Dynproc_exchange_map(root,0,port_id,&conn,conname,comm_ptr,
                                                                              &child_table_sz,&child_root,
                                                                              &child_addr_table,
                                                                              &child_node_table));
        MPIDI_CH4_NMI_OFI_CALL(fi_av_insert(MPIDI_Global.av,conname,1,&conn,0ULL,NULL),avmap);
        MPIDI_CH4_NMI_OFI_TABLE_INDEX_INCR();
        MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Dynproc_exchange_map(root,1,port_id,&conn,conname,comm_ptr,
                                                                              &child_table_sz,&child_root,
                                                                              &child_addr_table,
                                                                              &child_node_table));
        MPIDI_CH4_NMI_OFI_CALL(fi_av_remove(MPIDI_Global.av, &conn,1,0ULL),avmap);
    }

    /* Map the new address table */
    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Dynproc_bcast(root,comm_ptr,&child_root,
                                                                   &child_table_sz,
                                                                   &child_addr_table,
                                                                   &child_node_table));
    /* Now Create the New Intercomm */
    entries   = child_table_sz/MPIDI_Global.addrnamelen;
    MPIDI_CH4_NMI_OFI_MPI_CALL_POP(MPIDI_CH4_NMI_OFI_Dynproc_create_intercomm(port_name,
                                                                              child_addr_table,
                                                                              child_node_table,
                                                                              entries,
                                                                              comm_ptr,
                                                                              newcomm,
                                                                              1,
                                                                              (char *)"Accept"));
fn_exit:
    MPID_THREAD_CS_EXIT(POBJ,MPIDI_CH4_NMI_OFI_THREAD_SPAWN_MUTEX);
    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_COMM_CLOSE_PORT);
    return mpi_errno;

fn_fail:
    goto fn_exit;
}

#endif /* NETMOD_OFI_WIN_H_INCLUDED */
