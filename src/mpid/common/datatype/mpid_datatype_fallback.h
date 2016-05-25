/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#ifndef MPIR_DATATYPE_FALLBACK_H_INCLUDED
#define MPIR_DATATYPE_FALLBACK_H_INCLUDED

#include "mpidu_datatype.h"

#define MPID_Datatype_get_loopptr_macro MPIDU_Datatype_get_loopptr_macro
#define MPID_Datatype_get_loopsize_macro MPIDU_Datatype_get_loopsize_macro
#define MPID_Datatype_set_loopdepth_macro MPIDU_Datatype_set_loopdepth_macro
#define MPID_Datatype_set_loopptr_macro MPIDU_Datatype_set_loopptr_macro
#define MPID_Datatype_set_loopsize_macro MPIDU_Datatype_set_loopsize_macro

#define MPID_Datatype_free MPIDU_Datatype_free
#define MPID_Datatype_free_contents MPIDU_Datatype_free_contents
#define MPID_Datatype_set_contents MPIDU_Datatype_set_contents
#define MPID_Datatype_size_external32 MPIDU_Datatype_size_external32

/* MPID_Segment */
typedef struct DLOOP_Segment MPID_Segment; /* MPIDU_Segment */

#define MPID_Segment_init MPIDU_Segment_init
#define MPID_Segment_alloc MPIDU_Segment_alloc
#define MPID_Segment_free MPIDU_Segment_free
#define MPID_Segment_pack MPIDU_Segment_pack
#define MPID_Segment_unpack MPIDU_Segment_unpack
#define MPID_Segment_pack_external32 MPIDU_Segment_pack_external32
#define MPID_Segment_unpack_external32 MPIDU_Segment_unpack_external32

/* MPID_Type */
#define MPID_Type_access_contents MPIDU_Type_access_contents
#define MPID_Type_blockindexed MPIDU_Type_blockindexed
#define MPID_Type_commit MPIDU_Type_commit
#define MPID_Type_contiguous MPIDU_Type_contiguous
#define MPID_Type_create_pairtype MPIDU_Type_create_pairtype
#define MPID_Type_create_resized MPIDU_Type_create_resized
#define MPID_Type_dup MPIDU_Type_dup
#define MPID_Type_get_contents MPIDU_Type_get_contents
#define MPID_Type_get_envelope MPIDU_Type_get_envelope
#define MPID_Type_indexed MPIDU_Type_indexed
#define MPID_Type_release_contents MPIDU_Type_release_contents
#define MPID_Type_struct MPIDU_Type_struct
#define MPID_Type_vector MPIDU_Type_vector
#define MPID_Type_zerolen MPIDU_Type_zerolen

#endif /* MPiD_DATATYPE_FALLBACK_H_INCLUDED */
