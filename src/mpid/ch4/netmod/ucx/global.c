/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Mellanox Technologies Ltd.
 *  Copyright (C) Mellanox Technologies Ltd. 2016. ALL RIGHTS RESERVED
 */
#include <mpidimpl.h>
#include "impl.h"
#include "ucx_types.h"

MPIDI_CH4_NMI_UCX_Global_t MPIDI_CH4_NMI_UCX_Global = { 0 };

ucp_ep_h *MPIDI_CH4_NMI_UCX_eps = NULL;
