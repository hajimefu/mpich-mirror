/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 */

#ifndef MPIR_DATATYPE_H_INCLUDED
#define MPIR_DATATYPE_H_INCLUDED

/* This value should be set to greatest value used as the type index suffix in
 * the predefined handles.  That is, look at the last two hex digits of all
 * predefined datatype handles, take the greatest one, and convert it to decimal
 * here. */
/* FIXME calculating this value this way is foolish, we should make this more
 * automatic and less error prone */
/* FIXME: Given that this is relatively static, an adequate alternative is
   to provide a check that this value is valid. */
#define MPIR_DATATYPE_N_BUILTIN 69
#define MPIR_DTYPE_BEGINNING  0
#define MPIR_DTYPE_END       -1

/*S
  MPIR_Datatype_contents - Holds envelope and contents data for a given
                           datatype

  Notes:
  Space is allocated beyond the structure itself in order to hold the
  arrays of types, ints, and aints, in that order.

  S*/
typedef struct MPIR_Datatype_contents {
    int combiner;
    int nr_ints;
    int nr_aints;
    int nr_types;
    /* space allocated beyond structure used to store the types[],
     * ints[], and aints[], in that order.
     */
} MPIR_Datatype_contents;

/* Datatype Structure */
/*S
  MPIR_Datatype - Description of the MPID Datatype structure

  Notes:
  The 'ref_count' is needed for nonblocking operations such as
.vb
   MPI_Type_struct( ... , &newtype );
   MPI_Irecv( buf, 1000, newtype, ..., &request );
   MPI_Type_free( &newtype );
   ...
   MPI_Wait( &request, &status );
.ve

  Module:
  Datatype-DS

  Notes:

  Alternatives:
  The following alternatives for the layout of this structure were considered.
  Most were not chosen because any benefit in performance or memory
  efficiency was outweighed by the added complexity of the implementation.

  A number of fields contain only boolean inforation ('is_contig',
  'has_sticky_ub', 'has_sticky_lb', 'is_permanent', 'is_committed').  These
  could be combined and stored in a single bit vector.

  'MPI_Type_dup' could be implemented with a shallow copy, where most of the
  data fields, would not be copied into the new object created by
  'MPI_Type_dup'; instead, the new object could point to the data fields in
  the old object.  However, this requires more code to make sure that fields
  are found in the correct objects and that deleting the old object doesn't
  invalidate the dup'ed datatype.

  Originally we attempted to keep contents/envelope data in a non-optimized
  dataloop.  The subarray and darray types were particularly problematic,
  and eventually we decided it would be simpler to just keep contents/
  envelope data in arrays separately.

  Earlier versions of the ADI used a single API to change the 'ref_count',
  with each MPI object type having a separate routine.  Since reference
  count changes are always up or down one, and since all MPI objects
  are defined to have the 'ref_count' field in the same place, the current
  ADI3 API uses two routines, 'MPIR_Object_add_ref' and
  'MPIR_Object_release_ref', to increment and decrement the reference count.

  S*/
struct MPIR_Datatype {
    /* handle and ref_count are filled in by MPIR_Handle_obj_alloc() */
    MPIR_OBJECT_HEADER; /* adds handle and ref_count fields */

    /* basic parameters for datatype, accessible via MPI calls */
    MPI_Aint size;   /* MPI_Count could be 128 bits, so use MPI_Aint */
    MPI_Aint extent, ub, lb, true_ub, true_lb;

    /* chars affecting subsequent datatype processing and creation */
    MPI_Aint alignsize;
    int has_sticky_ub, has_sticky_lb;
    int is_permanent; /* non-zero if datatype is a predefined type */
    int is_committed;

    /* element information; used for accumulate and get elements
     * basic_type: describes basic type (predefined type). If the
     *             type is composed of the same basic type, it is
     *             set to that type, otherwise it is set to MPI_DATATYPE_NULL.
     * n_builtin_elements: refers to the number of builtin type elements.
     * builtin_element_size: refers to the size of builtin type. If the
     *                       type is composed of the same builtin type,
     *                       it is set to size of that type, otherwise it
     *                       is set to -1.
     */
    int      basic_type;
    MPI_Aint n_builtin_elements;
    MPI_Aint builtin_element_size;

    /* information on contiguity of type, for processing shortcuts.
     *
     * is_contig is non-zero only if N instances of the type would be
     * contiguous.
     */
    int is_contig;
    /* Upper bound on the number of contig blocks for one instance.
     * It is not trivial to calculate the *real* number of contig
     * blocks in the case where old datatype is non-contiguous
     */
    MPI_Aint max_contig_blocks;

    /* pointer to contents and envelope data for the datatype */
    MPIR_Datatype_contents *contents;

    /* dataloop members, including a pointer to the loop, the size in bytes,
     * and a depth used to verify that we can process it (limited stack depth
     */
    struct MPIR_Dataloop *dataloop; /* might be optimized for homogenous */
    MPI_Aint              dataloop_size;
    int                   dataloop_depth;
#if defined(MPID_HAS_HETERO) || 1
    struct MPIR_Dataloop *hetero_dloop; /* heterogeneous dataloop */
    MPI_Aint              hetero_dloop_size;
    int                   hetero_dloop_depth;
#endif /* MPID_HAS_HETERO */
    /* MPI-2 attributes and name */
    struct MPIR_Attribute *attributes;
    char                  name[MPI_MAX_OBJECT_NAME];

    /* not yet used; will be used to track what processes have cached
     * copies of this type.
     */
    int32_t cache_id;
    /* MPID_Lpidmask mask; */

    /* int (*free_fn)( struct MPIR_Datatype * ); */ /* Function to free this datatype */

    /* Other, device-specific information */
#ifdef MPID_DEV_DATATYPE_DECL
    MPID_DEV_DATATYPE_DECL
#endif
};

extern MPIR_Object_alloc_t MPIR_Datatype_mem;

#define MPIR_Datatype_add_ref(datatype_ptr) MPIR_Object_add_ref((datatype_ptr))

/* to be used only after MPIR_Datatype_valid_ptr(); the check on
 * err == MPI_SUCCESS ensures that we won't try to dereference the
 * pointer if something has already been detected as wrong.
 */
#define MPIR_Datatype_committed_ptr(ptr,err) do {               \
    if ((err == MPI_SUCCESS) && !((ptr)->is_committed))         \
        err = MPIR_Err_create_code(MPI_SUCCESS,                 \
                                   MPIR_ERR_RECOVERABLE,        \
                                   FCNAME,                      \
                                   __LINE__,                    \
                                   MPI_ERR_TYPE,                \
                                   "**dtypecommit",             \
                                   0);                          \
} while(0)

#define MPIR_Datatype_get_basic_size(a) (((a)&0x0000ff00)>>8)

#define MPIR_Datatype_get_basic_type(a,basic_type_) do {            \
    void *ptr;                                                      \
    switch (HANDLE_GET_KIND(a)) {                                   \
        case HANDLE_KIND_DIRECT:                                    \
            ptr = MPIR_Datatype_direct+HANDLE_INDEX(a);             \
            basic_type_ = ((MPIR_Datatype *) ptr)->basic_type;      \
            break;                                                  \
        case HANDLE_KIND_INDIRECT:                                  \
            ptr = ((MPIR_Datatype *)                                \
             MPIR_Handle_get_ptr_indirect(a,&MPIR_Datatype_mem));   \
            basic_type_ = ((MPIR_Datatype *) ptr)->basic_type;      \
            break;                                                  \
        case HANDLE_KIND_BUILTIN:                                   \
            basic_type_ = a;                                        \
            break;                                                  \
        case HANDLE_KIND_INVALID:                                   \
        default:                                                    \
         basic_type_ = 0;                                           \
         break;                                                     \
                                                                    \
    }                                                               \
    /* This macro returns the builtin type, if 'basic_type' is not  \
     * a builtin type, it must be a pair type composed of different \
     * builtin types, so we return MPI_DATATYPE_NULL here.          \
     */                                                             \
    if (HANDLE_GET_KIND(basic_type_) != HANDLE_KIND_BUILTIN)        \
        basic_type_ = MPI_DATATYPE_NULL;                            \
 } while(0)

#define MPIR_Datatype_get_ptr(a,ptr)   MPIR_Getb_ptr(Datatype,a,0x000000ff,ptr)

/* Note: Probably there is some clever way to build all of these from a macro.
 */
#define MPIR_Datatype_get_size_macro(a,size_) do {                     \
    void *ptr;                                                          \
    switch (HANDLE_GET_KIND(a)) {                                       \
        case HANDLE_KIND_DIRECT:                                        \
            ptr = MPIR_Datatype_direct+HANDLE_INDEX(a);                 \
            size_ = ((MPIR_Datatype *) ptr)->size;                      \
            break;                                                      \
        case HANDLE_KIND_INDIRECT:                                      \
            ptr = ((MPIR_Datatype *)                                    \
             MPIR_Handle_get_ptr_indirect(a,&MPIR_Datatype_mem));       \
            size_ = ((MPIR_Datatype *) ptr)->size;                      \
            break;                                                      \
        case HANDLE_KIND_BUILTIN:                                       \
            size_ = MPIR_Datatype_get_basic_size(a);                    \
            break;                                                      \
        case HANDLE_KIND_INVALID:                                       \
        default:                                                        \
         size_ = 0;                                                     \
         break;                                                         \
                                                                        \
    }                                                                   \
} while(0)

#define MPIR_Datatype_get_extent_macro(a,extent_) do {                  \
    void *ptr;                                                          \
    switch (HANDLE_GET_KIND(a)) {                                       \
        case HANDLE_KIND_DIRECT:                                        \
            ptr = MPIR_Datatype_direct+HANDLE_INDEX(a);                 \
            extent_ = ((MPIR_Datatype *) ptr)->extent;                  \
            break;                                                      \
        case HANDLE_KIND_INDIRECT:                                      \
            ptr = ((MPIR_Datatype *)                                    \
             MPIR_Handle_get_ptr_indirect(a,&MPIR_Datatype_mem));       \
            extent_ = ((MPIR_Datatype *) ptr)->extent;                  \
            break;                                                      \
        case HANDLE_KIND_INVALID:                                       \
        case HANDLE_KIND_BUILTIN:                                       \
        default:                                                        \
            extent_ = MPIR_Datatype_get_basic_size(a);  /* same as size */  \
            break;                                                      \
    }                                                                   \
} while(0)

/* helper macro: takes an MPI_Datatype handle value and returns TRUE in
 * (*is_config_) if the type is contiguous */
#define MPIR_Datatype_is_contig(dtype_, is_contig_)                            \
    do {                                                                       \
        if (HANDLE_GET_KIND(dtype_) == HANDLE_KIND_BUILTIN) {                  \
            *(is_contig_) = TRUE;                                              \
        }                                                                      \
        else {                                                                 \
            MPIR_Datatype *dtp_ = NULL;                                        \
            MPIR_Datatype_get_ptr((dtype_), dtp_);                             \
            *(is_contig_) = dtp_->is_contig;                                   \
        }                                                                      \
    } while (0)

/* MPIR_Datatype_release decrements the reference count on the MPIR_Datatype
 * and, if the refct is then zero, frees the MPIR_Datatype and associated
 * structures.
 */
#define MPIR_Datatype_release(datatype_ptr) do {                            \
    int inuse_;                                                             \
                                                                            \
    MPIR_Object_release_ref((datatype_ptr),&inuse_);                        \
    if (!inuse_) {                                                          \
        int lmpi_errno = MPI_SUCCESS;                                       \
     if (MPIR_Process.attr_free && datatype_ptr->attributes) {              \
         lmpi_errno = MPIR_Process.attr_free( datatype_ptr->handle,         \
                               &datatype_ptr->attributes );                 \
     }                                                                      \
     /* LEAVE THIS COMMENTED OUT UNTIL WE HAVE SOME USE FOR THE FREE_FN     \
     if (datatype_ptr->free_fn) {                                           \
         mpi_errno = (datatype_ptr->free_fn)( datatype_ptr );               \
          if (mpi_errno) {                                                  \
           MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPI_TYPE_FREE);                  \
           return MPIR_Err_return_comm( 0, FCNAME, mpi_errno );             \
          }                                                                 \
     } */                                                                   \
        if (lmpi_errno == MPI_SUCCESS) {                                    \
         MPIDU_Datatype_free(datatype_ptr);                                 \
        }                                                                   \
    }                                                                       \
} while(0)

#define MPIR_Datatype_valid_ptr(ptr,err) MPIR_Valid_ptr_class(Datatype,ptr,MPI_ERR_TYPE,err)

#define MPIR_Datatype_get_loopdepth_macro(a,depth_,hetero_) do {    \
    void *ptr;                                                      \
    switch (HANDLE_GET_KIND(a)) {                                   \
        case HANDLE_KIND_DIRECT:                                    \
            ptr = MPIR_Datatype_direct+HANDLE_INDEX(a);             \
            MPIDU_GET_FIELD(hetero_,depth_,_depth);                 \
            break;                                                  \
        case HANDLE_KIND_INDIRECT:                                  \
            ptr = ((MPIR_Datatype *)                                \
             MPIR_Handle_get_ptr_indirect(a,&MPIR_Datatype_mem));   \
            MPIDU_GET_FIELD(hetero_,depth_,_depth);                 \
            break;                                                  \
        case HANDLE_KIND_INVALID:                                   \
        case HANDLE_KIND_BUILTIN:                                   \
        default:                                                    \
            depth_ = 0;                                             \
            break;                                                  \
    }                                                               \
} while(0)


#define MPIR_Datatype_get_loopptr_macro(a,lptr_,hetero_) do {         \
    void *ptr;                                    \
    switch (HANDLE_GET_KIND(a)) {                      \
        case HANDLE_KIND_DIRECT:                       \
            ptr = MPIR_Datatype_direct+HANDLE_INDEX(a);               \
            MPIDU_GET_FIELD(hetero_,lptr_,);                             \
            break;                                \
        case HANDLE_KIND_INDIRECT:                     \
            ptr = ((MPIR_Datatype *)                        \
             MPIR_Handle_get_ptr_indirect(a,&MPIR_Datatype_mem));     \
            MPIDU_GET_FIELD(hetero_,lptr_,);                             \
            break;                                \
        case HANDLE_KIND_INVALID:                      \
        case HANDLE_KIND_BUILTIN:                      \
        default:                                  \
            lptr_ = 0;                                 \
            break;                                \
    }                                             \
} while(0)

#define MPIR_Datatype_get_loopsize_macro(a,depth_,hetero_) do {     \
    void *ptr;                                                      \
    switch (HANDLE_GET_KIND(a)) {                                   \
        case HANDLE_KIND_DIRECT:                                    \
            ptr = MPIR_Datatype_direct+HANDLE_INDEX(a);             \
            MPIDU_GET_FIELD(hetero_,depth_,_size);                  \
            break;                                                  \
        case HANDLE_KIND_INDIRECT:                                  \
            ptr = ((MPIR_Datatype *)                                \
             MPIR_Handle_get_ptr_indirect(a,&MPIR_Datatype_mem));   \
            MPIDU_GET_FIELD(hetero_,depth_,_size);                  \
            break;                                                  \
        case HANDLE_KIND_INVALID:                                   \
        case HANDLE_KIND_BUILTIN:                                   \
        default:                                                    \
            depth_ = 0;                                             \
            break;                                                  \
    }                                                               \
} while(0)

#define MPIR_Datatype_set_loopdepth_macro(a,depth_,hetero_) do {    \
    void *ptr;                                                      \
    switch (HANDLE_GET_KIND(a)) {                                   \
        case HANDLE_KIND_DIRECT:                                    \
            ptr = MPIR_Datatype_direct+HANDLE_INDEX(a);             \
            MPIDU_SET_FIELD(hetero_,depth_,_depth);                 \
            break;                                                  \
        case HANDLE_KIND_INDIRECT:                                  \
            ptr = ((MPIR_Datatype *)                                \
             MPIR_Handle_get_ptr_indirect(a,&MPIR_Datatype_mem));   \
            MPIDU_SET_FIELD(hetero_,depth_,_depth);                 \
            break;                                                  \
        case HANDLE_KIND_INVALID:                                   \
        case HANDLE_KIND_BUILTIN:                                   \
        default:                                                    \
            depth_ = 0;                                             \
            break;                                                  \
    }                                                               \
} while(0)

#define MPIR_Datatype_set_loopptr_macro(a,lptr_,hetero_) do {       \
    void *ptr;                                                      \
    switch (HANDLE_GET_KIND(a)) {                                   \
        case HANDLE_KIND_DIRECT:                                    \
            ptr = MPIR_Datatype_direct+HANDLE_INDEX(a);             \
            MPIDU_SET_FIELD(hetero_,lptr_,);                        \
            break;                                                  \
        case HANDLE_KIND_INDIRECT:                                  \
            ptr = ((MPIR_Datatype *)                                \
             MPIR_Handle_get_ptr_indirect(a,&MPIR_Datatype_mem));   \
            MPIDU_SET_FIELD(hetero_,lptr_,);                        \
            break;                                                  \
        case HANDLE_KIND_INVALID:                                   \
        case HANDLE_KIND_BUILTIN:                                   \
        default:                                                    \
            lptr_ = 0;                                              \
            break;                                                  \
    }                                                               \
} while(0)

#define MPIR_Datatype_set_loopsize_macro(a,depth_,hetero_) do {     \
    void *ptr;                                                      \
    switch (HANDLE_GET_KIND(a)) {                                   \
        case HANDLE_KIND_DIRECT:                                    \
            ptr = MPIR_Datatype_direct+HANDLE_INDEX(a);             \
            MPIDU_SET_FIELD(hetero_,depth_,_size);                  \
            break;                                                  \
        case HANDLE_KIND_INDIRECT:                                  \
            ptr = ((MPIR_Datatype *)                                \
             MPIR_Handle_get_ptr_indirect(a,&MPIR_Datatype_mem));   \
            MPIDU_SET_FIELD(hetero_,depth_,_size);                  \
            break;                                                  \
        case HANDLE_KIND_INVALID:                                   \
        case HANDLE_KIND_BUILTIN:                                   \
        default:                                                    \
            depth_ = 0;                                             \
            break;                                                  \
    }                                                               \
} while(0)

/* This routine is used to install an attribute free routine for datatypes
   at finalize-time */
void MPII_Datatype_attr_finalize( void );

#define MPIR_DATATYPE_IS_PREDEFINED(type) \
    ((HANDLE_GET_KIND(type) == HANDLE_KIND_BUILTIN) || \
     (type == MPI_FLOAT_INT) || (type == MPI_DOUBLE_INT) || \
     (type == MPI_LONG_INT) || (type == MPI_SHORT_INT) || \
     (type == MPI_LONG_DOUBLE_INT))

int MPIR_Get_elements_x_impl(const MPI_Status *status, MPI_Datatype datatype, MPI_Count *elements);
int MPIR_Status_set_elements_x_impl(MPI_Status *status, MPI_Datatype datatype, MPI_Count count);
void MPIR_Type_get_extent_x_impl(MPI_Datatype datatype, MPI_Count *lb, MPI_Count *extent);
void MPIR_Type_get_true_extent_x_impl(MPI_Datatype datatype, MPI_Count *true_lb, MPI_Count *true_extent);
int MPIR_Type_size_x_impl(MPI_Datatype datatype, MPI_Count *size);

void MPIR_Get_count_impl(const MPI_Status *status, MPI_Datatype datatype, int *count);
int MPIR_Type_commit_impl(MPI_Datatype *datatype);
int MPIR_Type_create_struct_impl(int count,
                                 const int array_of_blocklengths[],
                                 const MPI_Aint array_of_displacements[],
                                 const MPI_Datatype array_of_types[],
                                 MPI_Datatype *newtype);
int MPIR_Type_create_indexed_block_impl(int count,
                                        int blocklength,
                                        const int array_of_displacements[],
                                        MPI_Datatype oldtype,
                                        MPI_Datatype *newtype);
int MPIR_Type_create_hindexed_block_impl(int count, int blocklength,
                                         const MPI_Aint array_of_displacements[],
                                         MPI_Datatype oldtype, MPI_Datatype *newtype);
int MPIR_Type_contiguous_impl(int count,
                              MPI_Datatype old_type,
                              MPI_Datatype *new_type_p);
int MPIR_Type_contiguous_x_impl(MPI_Count count,
                              MPI_Datatype old_type,
                              MPI_Datatype *new_type_p);
void MPIR_Type_get_extent_impl(MPI_Datatype datatype, MPI_Aint *lb, MPI_Aint *extent);
void MPIR_Type_get_true_extent_impl(MPI_Datatype datatype, MPI_Aint *true_lb, MPI_Aint *true_extent);
void MPIR_Type_get_envelope_impl(MPI_Datatype datatype, int *num_integers, int *num_addresses,
                                 int *num_datatypes, int *combiner);
int MPIR_Type_hvector_impl(int count, int blocklen, MPI_Aint stride, MPI_Datatype old_type, MPI_Datatype *newtype_p);
int MPIR_Type_indexed_impl(int count, const int blocklens[], const int indices[],
                           MPI_Datatype old_type, MPI_Datatype *newtype);
void MPIR_Type_free_impl(MPI_Datatype *datatype);
int MPIR_Type_vector_impl(int count, int blocklength, int stride, MPI_Datatype old_type, MPI_Datatype *newtype_p);
int MPIR_Type_struct_impl(int count, const int blocklens[], const MPI_Aint indices[], const MPI_Datatype old_types[], MPI_Datatype *newtype);
int MPIR_Pack_impl(const void *inbuf, MPI_Aint incount, MPI_Datatype datatype, void *outbuf, MPI_Aint outcount, MPI_Aint *position);
void MPIR_Pack_size_impl(int incount, MPI_Datatype datatype, MPI_Aint *size);
int MPIR_Unpack_impl(const void *inbuf, MPI_Aint insize, MPI_Aint *position,
                     void *outbuf, int outcount, MPI_Datatype datatype);
void MPIR_Type_lb_impl(MPI_Datatype datatype, MPI_Aint *displacement);

#endif /* MPIR_DATATYPE_H_INCLUDED */
