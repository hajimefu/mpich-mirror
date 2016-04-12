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
/* ch4 shm functions */
#ifndef SHM_IMPL_PROTOTYPES_H_INCLUDED
#define SHM_IMPL_PROTOTYPES_H_INCLUDED

#ifndef SHM_DIRECT
#ifndef SHM_DISABLE_INLINES
#define USE_SHM_INLINES
#include "ch4_shm_api.h"
#undef USE_SHM_INLINES

#endif /* SHM_DISABLE_INLINES  */

#else

#define __shm_direct_stub__     0
#define __shm_direct_simple__   1

#if SHM_DIRECT==__shm_direct_stub__
#include "../stub/ch4_shm_direct.h"
#elif SHM_DIRECT==__shm_direct_simple__
#include "../simple/ch4_shm_direct.h"
#else
#error "No direct shm included"
#endif


#endif /* SHM_DIRECT           */

#endif
