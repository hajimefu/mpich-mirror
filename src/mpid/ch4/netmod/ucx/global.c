/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
#include <mpidimpl.h>
#include "impl.h"
#include "ucx_types.h"

MPIDI_CH4_NMI_UCX_Global_t MPIDI_CH4_NMI_UCX_Global = { 0 };

ucp_ep_h *MPIDI_CH4_NMI_UCX_eps = NULL;
