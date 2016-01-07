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

#ifndef NETMOD_OFI_PRE_H_INCLUDED
#define NETMOD_OFI_PRE_H_INCLUDED

#include <mpi.h>
#include "mpihandlemem.h"
#include <rdma/fabric.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_tagged.h>
#include <rdma/fi_rma.h>
#include <rdma/fi_atomic.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_errno.h>
#include "ofi_pre_common.h"

typedef struct fid_ep *fid_base_ep_t;
typedef struct fid_stx *fid_stx_t;
typedef struct fid_ep *fid_srx_t;
typedef struct fid_ep *fid_ep_t;
typedef struct fid_fabric *fid_fabric_t;
typedef struct fid_domain *fid_domain_t;
typedef struct fid_cq *fid_cq_t;
typedef struct fid_av *fid_av_t;
typedef struct fid_mr *fid_mr_t;
typedef struct fid_cntr *fid_cntr_t;
typedef struct fi_fabric fabric_t;
typedef struct fi_msg msg_t;
typedef struct fi_info info_t;
typedef struct fi_cq_attr cq_attr_t;
typedef struct fi_cntr_attr cntr_attr_t;
typedef struct fi_av_attr av_attr_t;
typedef struct fi_domain_attr domain_attr_t;
typedef struct fi_tx_attr tx_attr_t;
typedef struct fi_rx_attr rx_attr_t;
typedef struct fi_cq_tagged_entry cq_tagged_entry_t;
typedef struct fi_cq_err_entry cq_err_entry_t;
typedef struct fi_context context_t;
typedef struct fi_info_addr info_addr_t;
typedef struct fi_msg_tagged msg_tagged_t;
typedef struct fi_rma_ioc rma_ioc_t;
typedef struct fi_rma_iov rma_iov_t;
typedef struct fi_ioc ioc_t;
typedef struct fi_msg_rma msg_rma_t;
typedef struct fi_msg_atomic msg_atomic_t;
typedef enum   fi_op fi_op_t;
typedef enum   fi_datatype fi_datatype_t;

struct MPID_Comm;
typedef struct {
    context_t            context;
    int                  event_id;
    int                  util_id;
    struct MPID_Comm    *util_comm;
    MPI_Datatype         datatype;
    struct MPID_Segment *segment_ptr;
    char                *pack_buffer;
    /* persistent send fields */
    union {
        struct {
            int   type;
            int   rank;
            int   tag;
            int   count;
            void *buf;
        } persist;
        struct iovec iov;
    }util;
} MPIDI_CH4_NM_ofi_request_t;

#endif
