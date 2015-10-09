/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
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
