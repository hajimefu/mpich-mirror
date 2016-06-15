/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#ifndef MPIDU_DATATYPE_H
#define MPIDU_DATATYPE_H

#include "mpiimpl.h"
#include "mpidu_dataloop.h"
#include "mpir_objects.h"

/* NOTE: 
 * - struct MPIDU_Dataloop and MPIDU_Segment are defined in 
 *   src/mpid/common/datatype/mpidu_dataloop.h (and gen_dataloop.h).
 * - MPIR_Object_alloc_t is defined in src/include/mpihandle.h
 */

void MPIDU_Dataloop_free(struct MPIDU_Dataloop **dataloop);

void MPIDU_Segment_flatten(struct DLOOP_Segment *segp,
			  DLOOP_Offset first,
			  DLOOP_Offset *lastp,
			  DLOOP_Offset *offp,
			  DLOOP_Size *sizep,
			  DLOOP_Offset *lengthp);

void MPIDU_Dataloop_update(struct DLOOP_Dataloop *dataloop,
			  MPI_Aint ptrdiff);

/* end of file */
#endif
