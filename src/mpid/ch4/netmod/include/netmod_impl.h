/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
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
#define __netmod_direct_am_ofi__ 3

#if NETMOD_DIRECT==__netmod_direct_stub__
#include "../stub/netmod_direct.h"
#elif NETMOD_DIRECT==__netmod_direct_ofi__
#include "../ofi/netmod_direct.h"
#elif NETMOD_DIRECT==__netmod_direct_shm__
#include "../shm/netmod_direct.h"
#elif NETMOD_DIRECT==__netmod_direct_am_ofi__
#include "../am_ofi/netmod_direct.h"
#else
#error "No direct netmod included"
#endif
#endif /* NETMOD_DIRECT           */

#endif
