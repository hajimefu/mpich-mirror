/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#ifndef MPIR_DATATYPE_FALLBACK_H_INCLUDED
#define MPIR_DATATYPE_FALLBACK_H_INCLUDED

#include "mpidu_datatype.h"

/* MPID_Type */
#define MPID_Type_dup MPIDU_Type_dup
#define MPID_Type_get_contents MPIDU_Type_get_contents
#define MPID_Type_get_envelope MPIDU_Type_get_envelope
#define MPID_Type_indexed MPIDU_Type_indexed
#define MPID_Type_struct MPIDU_Type_struct
#define MPID_Type_vector MPIDU_Type_vector
#define MPID_Type_zerolen MPIDU_Type_zerolen

#endif /* MPiD_DATATYPE_FALLBACK_H_INCLUDED */
