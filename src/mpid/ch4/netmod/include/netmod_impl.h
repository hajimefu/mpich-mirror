/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
/* ch4 netmod functions */
#ifndef NETMOD_IMPL_PROTOTYPES_H_INCLUDED
#define NETMOD_IMPL_PROTOTYPES_H_INCLUDED


#ifndef NETMOD_DIRECT
#ifndef NETMOD_DISABLE_INLINES
#define USE_NETMOD_INLINES
#include "netmod_api.h"
#undef USE_NETMOD_INLINES

#endif /* NETMOD_DISABLE_INLINES  */

#else
#define __netmod_direct_stub__   0
#define __netmod_direct_ofi__    1
#define __netmod_direct_shm__    2
#define __netmod_direct_ucx__    3
#define __netmod_direct_portals4__ 4

#if NETMOD_DIRECT==__netmod_direct_stub__
#include "../stub/netmod_direct.h"
#elif NETMOD_DIRECT==__netmod_direct_ofi__
#include "../ofi/netmod_direct.h"
#elif NETMOD_DIRECT==__netmod_direct_shm__
#include "../shm/netmod_direct.h"
#elif NETMOD_DIRECT==__netmod_direct_ucx__
#include "../ucx/netmod_direct.h"
#elif NETMOD_DIRECT==__netmod_direct_portals4__
#include "../portals4/netmod_direct.h"
#else
#error "No direct netmod included"
#endif
#endif /* NETMOD_DIRECT           */

#endif
