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

/* helper macro: takes an MPI_Datatype handle value and returns true_lb in
 * (*true_lb_) */
#define MPIDU_Datatype_get_true_lb(dtype_, true_lb_)                            \
    do {                                                                       \
        if (HANDLE_GET_KIND(dtype_) == HANDLE_KIND_BUILTIN) {                  \
            *(true_lb_) = 0;                                                   \
        }                                                                      \
        else {                                                                 \
            MPIR_Datatype *dtp_ = NULL;                                        \
            MPIR_Datatype_get_ptr((dtype_), dtp_);                             \
            *(true_lb_) = dtp_->true_lb;                                       \
        }                                                                      \
    } while (0)

/* Datatype functions */
int MPIR_Type_dup(MPI_Datatype oldtype,
		  MPI_Datatype *newtype);

int MPIR_Type_struct(int count,
		     const int *blocklength_array,
		     const MPI_Aint *displacement_array,
		     const MPI_Datatype *oldtype_array,
		     MPI_Datatype *newtype);

int MPIR_Type_indexed(int count,
		      const int *blocklength_array,
		      const void *displacement_array,
		      int dispinbytes,
		      MPI_Datatype oldtype,
		      MPI_Datatype *newtype);

int MPIR_Type_vector(int count,
		     int blocklength,
		     MPI_Aint stride,
		     int strideinbytes,
		     MPI_Datatype oldtype,
		     MPI_Datatype *newtype);

int MPIR_Type_contiguous(int count,
			 MPI_Datatype oldtype,
			 MPI_Datatype *newtype);

int MPII_Type_zerolen(MPI_Datatype *newtype);

int MPIR_Type_create_resized(MPI_Datatype oldtype,
			     MPI_Aint lb,
			     MPI_Aint extent,
			     MPI_Datatype *newtype);

int MPIR_Type_get_envelope(MPI_Datatype datatype,
			   int *num_integers,
			   int *num_addresses,
			   int *num_datatypes,
			   int *combiner);

int MPIR_Type_get_contents(MPI_Datatype datatype, 
			   int max_integers, 
			   int max_addresses, 
			   int max_datatypes, 
			   int array_of_integers[], 
			   MPI_Aint array_of_addresses[], 
			   MPI_Datatype array_of_datatypes[]);

int MPIR_Type_create_pairtype(MPI_Datatype datatype,
                              MPIR_Datatype *new_dtp);

/* internal debugging functions */
void MPIDI_Datatype_printf(MPI_Datatype type,
			   int depth,
			   MPI_Aint displacement,
			   int blocklength,
			   int header);

/* Dataloop functions */
void MPIDU_Dataloop_copy(void *dest,
			void *src,
			MPI_Aint size);

void MPIDU_Dataloop_print(struct MPIDU_Dataloop *dataloop,
			 int depth);

void MPIDU_Dataloop_alloc(int kind,
			 MPI_Aint count,
			 DLOOP_Dataloop **new_loop_p,
			 MPI_Aint *new_loop_sz_p);

void MPIDU_Dataloop_alloc_and_copy(int kind,
				  MPI_Aint count,
				  struct DLOOP_Dataloop *old_loop,
				  MPI_Aint old_loop_sz,
				  struct DLOOP_Dataloop **new_loop_p,
				  MPI_Aint *new_loop_sz_p);
void MPIDU_Dataloop_struct_alloc(MPI_Aint count,
				MPI_Aint old_loop_sz,
				int basic_ct,
				DLOOP_Dataloop **old_loop_p,
				DLOOP_Dataloop **new_loop_p,
				MPI_Aint *new_loop_sz_p);
void MPIDU_Dataloop_dup(DLOOP_Dataloop *old_loop,
		       MPI_Aint old_loop_sz,
		       DLOOP_Dataloop **new_loop_p);
void MPIDU_Dataloop_free(struct MPIDU_Dataloop **dataloop);

/* Segment functions specific to MPICH */
void MPIR_Segment_pack_vector(struct DLOOP_Segment *segp,
			      DLOOP_Offset first,
			      DLOOP_Offset *lastp,
			      DLOOP_VECTOR *vector,
			      int *lengthp);

void MPIR_Segment_unpack_vector(struct DLOOP_Segment *segp,
				DLOOP_Offset first,
				DLOOP_Offset *lastp,
				DLOOP_VECTOR *vector,
				int *lengthp);

void MPIDU_Segment_flatten(struct DLOOP_Segment *segp,
			  DLOOP_Offset first,
			  DLOOP_Offset *lastp,
			  DLOOP_Offset *offp,
			  DLOOP_Size *sizep,
			  DLOOP_Offset *lengthp);

/* misc */
void MPIDI_Datatype_get_contents_aints(MPIDU_Datatype_contents *cp,
				       MPI_Aint *user_aints);
void MPIDI_Datatype_get_contents_types(MPIDU_Datatype_contents *cp,
				       MPI_Datatype *user_types);
void MPIDI_Datatype_get_contents_ints(MPIDU_Datatype_contents *cp,
				      int *user_ints);

void MPIDU_Dataloop_update(struct DLOOP_Dataloop *dataloop,
			  MPI_Aint ptrdiff);

/* end of file */
#endif
