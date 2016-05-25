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

/* MPIDU_Datatype_get_basic_id() is useful for creating and indexing into arrays
   that store data on a per-basic type basis */
#define MPIDU_Datatype_get_basic_id(a) ((a)&0x000000ff)

/*
 * The following macro allows us to reference either the regular or 
 * hetero value for the 3 fields (NULL,_size,_depth) in the
 * MPIR_Datatype structure.  This is used in the many
 * macros that access fields of the datatype.  We need this macro
 * to simplify the definition of the other macros in the case where
 * MPID_HAS_HETERO is *not* defined.
 */
#if defined(MPID_HAS_HETERO) || 1
#define MPIDU_GET_FIELD(hetero_,value_,fieldname_) do {                          \
        if (hetero_ != MPIDU_DATALOOP_HETEROGENEOUS)                             \
            value_ = ((MPIR_Datatype *)ptr)->dataloop##fieldname_;              \
        else value_ = ((MPIR_Datatype *) ptr)->hetero_dloop##fieldname_;        \
    } while(0)
#else
#define MPIDU_GET_FIELD(hetero_,value_,fieldname_) \
      value_ = ((MPIR_Datatype *)ptr)->dataloop##fieldname_
#endif

#if defined(MPID_HAS_HETERO) || 1
#define MPIDU_SET_FIELD(hetero_,value_,fieldname_) do {                          \
        if (hetero_ != MPIDU_DATALOOP_HETEROGENEOUS)                             \
            ((MPIR_Datatype *)ptr)->dataloop##fieldname_ = value_;              \
        else ((MPIR_Datatype *) ptr)->hetero_dloop##fieldname_ = value_;        \
    } while(0)
#else
#define MPIDU_SET_FIELD(hetero_,value_,fieldname_) \
    ((MPIR_Datatype *)ptr)->dataloop##fieldname_ = value_
#endif

#define MPIDU_Datatype_set_loopsize_macro(a,depth_,hetero_) do {         \
    void *ptr;								\
    switch (HANDLE_GET_KIND(a)) {					\
        case HANDLE_KIND_DIRECT:					\
            ptr = MPIR_Datatype_direct+HANDLE_INDEX(a);			\
            MPIDU_SET_FIELD(hetero_,depth_,_size);                       \
            break;							\
        case HANDLE_KIND_INDIRECT:					\
            ptr = ((MPIR_Datatype *)					\
		   MPIR_Handle_get_ptr_indirect(a,&MPIR_Datatype_mem));	\
            MPIDU_SET_FIELD(hetero_,depth_,_size);                       \
            break;							\
        case HANDLE_KIND_INVALID:					\
        case HANDLE_KIND_BUILTIN:					\
        default:							\
            depth_ = 0;                                                 \
            break;							\
    }                                                                   \
} while(0)

/* LB/UB calculation helper macros */

/* MPIDU_DATATYPE_CONTIG_LB_UB()
 *
 * Determines the new LB and UB for a block of old types given the
 * old type's LB, UB, and extent, and a count of these types in the
 * block.
 *
 * Note: if the displacement is non-zero, the MPIDU_DATATYPE_BLOCK_LB_UB()
 * should be used instead (see below).
 */
#define MPIDU_DATATYPE_CONTIG_LB_UB(cnt_,		\
				   old_lb_,		\
				   old_ub_,		\
				   old_extent_,	\
				   lb_,		\
				   ub_)		\
do {							\
    if (cnt_ == 0) {					\
	lb_ = old_lb_;				\
	ub_ = old_ub_;				\
    }							\
    else if (old_ub_ >= old_lb_) {			\
        lb_ = old_lb_;				\
        ub_ = old_ub_ + (old_extent_) * (cnt_ - 1);	\
    }							\
    else /* negative extent */ {			\
	lb_ = old_lb_ + (old_extent_) * (cnt_ - 1);	\
	ub_ = old_ub_;				\
    }                                                   \
} while(0)

/* MPIDU_DATATYPE_VECTOR_LB_UB()
 *
 * Determines the new LB and UB for a vector of blocks of old types
 * given the old type's LB, UB, and extent, and a count, stride, and
 * blocklen describing the vectorization.
 */
#define MPIDU_DATATYPE_VECTOR_LB_UB(cnt_,			\
				   stride_,			\
				   blklen_,			\
				   old_lb_,			\
				   old_ub_,			\
				   old_extent_,		\
				   lb_,			\
				   ub_)			\
do {								\
    if (cnt_ == 0 || blklen_ == 0) {				\
	lb_ = old_lb_;					\
	ub_ = old_ub_;					\
    }								\
    else if (stride_ >= 0 && (old_extent_) >= 0) {		\
	lb_ = old_lb_;					\
	ub_ = old_ub_ + (old_extent_) * ((blklen_) - 1) +	\
	    (stride_) * ((cnt_) - 1);				\
    }								\
    else if (stride_ < 0 && (old_extent_) >= 0) {		\
	lb_ = old_lb_ + (stride_) * ((cnt_) - 1);		\
	ub_ = old_ub_ + (old_extent_) * ((blklen_) - 1);	\
    }								\
    else if (stride_ >= 0 && (old_extent_) < 0) {		\
	lb_ = old_lb_ + (old_extent_) * ((blklen_) - 1);	\
	ub_ = old_ub_ + (stride_) * ((cnt_) - 1);		\
    }								\
    else {							\
	lb_ = old_lb_ + (old_extent_) * ((blklen_) - 1) +	\
	    (stride_) * ((cnt_) - 1);				\
	ub_ = old_ub_;					\
    }								\
} while(0)

/* MPIDU_DATATYPE_BLOCK_LB_UB()
 *
 * Determines the new LB and UB for a block of old types given the LB,
 * UB, and extent of the old type as well as a new displacement and count
 * of types.
 *
 * Note: we need the extent here in addition to the lb and ub because the
 * extent might have some padding in it that we need to take into account.
 */
#define MPIDU_DATATYPE_BLOCK_LB_UB(cnt_,				\
				  disp_,				\
				  old_lb_,				\
				  old_ub_,				\
				  old_extent_,				\
				  lb_,					\
				  ub_)					\
do {									\
    if (cnt_ == 0) {							\
	lb_ = old_lb_ + (disp_);					\
	ub_ = old_ub_ + (disp_);					\
    }									\
    else if (old_ub_ >= old_lb_) {					\
        lb_ = old_lb_ + (disp_);					\
        ub_ = old_ub_ + (disp_) + (old_extent_) * ((cnt_) - 1);	\
    }									\
    else /* negative extent */ {					\
	lb_ = old_lb_ + (disp_) + (old_extent_) * ((cnt_) - 1);	\
	ub_ = old_ub_ + (disp_);					\
    }									\
} while(0)

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
int MPIDU_Type_commit(MPI_Datatype *type);

int MPIDU_Type_dup(MPI_Datatype oldtype,
		  MPI_Datatype *newtype);

int MPIDU_Type_struct(int count,
		     const int *blocklength_array,
		     const MPI_Aint *displacement_array,
		     const MPI_Datatype *oldtype_array,
		     MPI_Datatype *newtype);

int MPIDU_Type_indexed(int count,
		      const int *blocklength_array,
		      const void *displacement_array,
		      int dispinbytes,
		      MPI_Datatype oldtype,
		      MPI_Datatype *newtype);

int MPIDU_Type_blockindexed(int count,
			   int blocklength,
			   const void *displacement_array,
			   int dispinbytes,
			   MPI_Datatype oldtype,
			   MPI_Datatype *newtype);

int MPIDU_Type_vector(int count,
		     int blocklength,
		     MPI_Aint stride,
		     int strideinbytes,
		     MPI_Datatype oldtype,
		     MPI_Datatype *newtype);

int MPIDU_Type_contiguous(int count,
			 MPI_Datatype oldtype,
			 MPI_Datatype *newtype);

int MPIDU_Type_zerolen(MPI_Datatype *newtype);

int MPIDU_Type_create_resized(MPI_Datatype oldtype,
			     MPI_Aint lb,
			     MPI_Aint extent,
			     MPI_Datatype *newtype);

int MPIDU_Type_get_envelope(MPI_Datatype datatype,
			   int *num_integers,
			   int *num_addresses,
			   int *num_datatypes,
			   int *combiner);

int MPIDU_Type_get_contents(MPI_Datatype datatype, 
			   int max_integers, 
			   int max_addresses, 
			   int max_datatypes, 
			   int array_of_integers[], 
			   MPI_Aint array_of_addresses[], 
			   MPI_Datatype array_of_datatypes[]);

int MPIDU_Type_create_pairtype(MPI_Datatype datatype,
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
void MPIDU_Segment_pack_vector(struct DLOOP_Segment *segp,
			      DLOOP_Offset first,
			      DLOOP_Offset *lastp,
			      DLOOP_VECTOR *vector,
			      int *lengthp);

void MPIDU_Segment_unpack_vector(struct DLOOP_Segment *segp,
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
int MPIDU_Datatype_set_contents(struct MPIR_Datatype *ptr,
			       int combiner,
			       int nr_ints,
			       int nr_aints,
			       int nr_types,
			       int *ints,
			       const MPI_Aint *aints,
			       const MPI_Datatype *types);

void MPIDU_Datatype_free_contents(struct MPIR_Datatype *ptr);
void MPIDI_Datatype_get_contents_aints(MPIDU_Datatype_contents *cp,
				       MPI_Aint *user_aints);
void MPIDI_Datatype_get_contents_types(MPIDU_Datatype_contents *cp,
				       MPI_Datatype *user_types);
void MPIDI_Datatype_get_contents_ints(MPIDU_Datatype_contents *cp,
				      int *user_ints);

void MPIDU_Datatype_free(struct MPIR_Datatype *ptr);

void MPIDU_Dataloop_update(struct DLOOP_Dataloop *dataloop,
			  MPI_Aint ptrdiff);

int MPIR_Type_flatten(MPI_Datatype type,
		      MPI_Aint *off_array,
		      DLOOP_Size *size_array,
		      MPI_Aint *array_len_p);

void MPIDU_Segment_pack_external32(struct DLOOP_Segment *segp,
				  DLOOP_Offset first,
				  DLOOP_Offset *lastp, 
				  void *pack_buffer);

void MPIDU_Segment_unpack_external32(struct DLOOP_Segment *segp,
				    DLOOP_Offset first,
				    DLOOP_Offset *lastp,
				    DLOOP_Buffer unpack_buffer);

MPI_Aint MPIDU_Datatype_size_external32(MPI_Datatype type);
MPI_Aint MPIDI_Datatype_get_basic_size_external32(MPI_Datatype el_type);

/* debugging helper functions */
char *MPIDU_Datatype_builtin_to_string(MPI_Datatype type);
char *MPIDU_Datatype_combiner_to_string(int combiner);
void MPIDU_Datatype_debug(MPI_Datatype type, int array_ct);

/* contents accessor functions */
void MPIDU_Type_access_contents(MPI_Datatype type,
			       int **ints_p,
			       MPI_Aint **aints_p,
			       MPI_Datatype **types_p);
void MPIDU_Type_release_contents(MPI_Datatype type,
				int **ints_p,
				MPI_Aint **aints_p,
				MPI_Datatype **types_p);

/* end of file */
#endif
