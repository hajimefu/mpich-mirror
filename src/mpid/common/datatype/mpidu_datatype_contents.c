/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */

/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include <mpiimpl.h>
#include <mpidu_dataloop.h>
#include <stdlib.h>
#include <limits.h>

void MPIDI_Datatype_get_contents_ints(MPIR_Datatype_contents *cp,
				      int *user_ints)
{
    char *ptr;
    int align_sz = 8, epsilon;
    int struct_sz, types_sz;

#ifdef HAVE_MAX_STRUCT_ALIGNMENT
    if (align_sz > HAVE_MAX_STRUCT_ALIGNMENT) {
	align_sz = HAVE_MAX_STRUCT_ALIGNMENT;
    }
#endif

    struct_sz = sizeof(MPIR_Datatype_contents);
    types_sz  = cp->nr_types * sizeof(MPI_Datatype);

    /* pad the struct, types, and ints before we allocate.
     *
     * note: it's not necessary that we pad the aints,
     *       because they are last in the region.
     */
    if ((epsilon = struct_sz % align_sz)) {
	struct_sz += align_sz - epsilon;
    }
    if ((epsilon = types_sz % align_sz)) {
	types_sz += align_sz - epsilon;
    }

    ptr = ((char *) cp) + struct_sz + types_sz;
    MPIR_Memcpy(user_ints, ptr, cp->nr_ints * sizeof(int));

    return;
}

void MPIDI_Datatype_get_contents_aints(MPIR_Datatype_contents *cp,
				       MPI_Aint *user_aints)
{
    char *ptr;
    int align_sz = 8, epsilon;
    int struct_sz, ints_sz, types_sz;

#ifdef HAVE_MAX_STRUCT_ALIGNMENT
    if (align_sz > HAVE_MAX_STRUCT_ALIGNMENT) {
	align_sz = HAVE_MAX_STRUCT_ALIGNMENT;
    }
#endif

    struct_sz = sizeof(MPIR_Datatype_contents);
    types_sz  = cp->nr_types * sizeof(MPI_Datatype);
    ints_sz   = cp->nr_ints * sizeof(int);

    /* pad the struct, types, and ints before we allocate.
     *
     * note: it's not necessary that we pad the aints,
     *       because they are last in the region.
     */
    if ((epsilon = struct_sz % align_sz)) {
	struct_sz += align_sz - epsilon;
    }
    if ((epsilon = types_sz % align_sz)) {
	types_sz += align_sz - epsilon;
    }
    if ((epsilon = ints_sz % align_sz)) {
	ints_sz += align_sz - epsilon;
    }

    ptr = ((char *) cp) + struct_sz + types_sz + ints_sz;
    MPIR_Memcpy(user_aints, ptr, cp->nr_aints * sizeof(MPI_Aint));

    return;
}

void MPIDI_Datatype_get_contents_types(MPIR_Datatype_contents *cp,
				       MPI_Datatype *user_types)
{
    char *ptr;
    int align_sz = 8, epsilon;
    int struct_sz;

#ifdef HAVE_MAX_STRUCT_ALIGNMENT
    if (align_sz > HAVE_MAX_STRUCT_ALIGNMENT) {
	align_sz = HAVE_MAX_STRUCT_ALIGNMENT;
    }
#endif

    struct_sz = sizeof(MPIR_Datatype_contents);

    /* pad the struct, types, and ints before we allocate.
     *
     * note: it's not necessary that we pad the aints,
     *       because they are last in the region.
     */
    if ((epsilon = struct_sz % align_sz)) {
	struct_sz += align_sz - epsilon;
    }

    ptr = ((char *) cp) + struct_sz;
    MPIR_Memcpy(user_types, ptr, cp->nr_types * sizeof(MPI_Datatype));

    return;
}
