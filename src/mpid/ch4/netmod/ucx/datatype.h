/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 */
#ifndef NETMOD_UCX_DATAYPE_H_INCLUDED
#define NETMOD_UCX_DATAYPE_H_INCLUDED

#include "impl.h"
#include "ucx_types.h"
#include <ucp/api/ucp.h>
struct MPIDI_CH4_NMI_UCX_pack_state{

    MPID_Segment *segment_ptr;
    MPI_Aint packsize;
};

static inline void* MPIDI_CH4_NMI_UCX_Start_pack(void *context, const void *buffer, size_t count){

    MPI_Datatype *datatype = (MPI_Datatype*) context;
    MPID_Segment *segment_ptr;
    struct MPIDI_CH4_NMI_UCX_pack_state *state;
    MPI_Aint packsize;
    state = MPL_malloc(sizeof(struct MPIDI_CH4_NMI_UCX_pack_state));
    segment_ptr = MPID_Segment_alloc();
    MPIR_Pack_size_impl(count, *datatype, &packsize);
/* Todo: Add error handling */
    MPID_Segment_init(buffer, count, *datatype, segment_ptr, 1);
    state->packsize = packsize;
    state->segment_ptr = segment_ptr;
    return (void*) state;
}

static inline void* MPIDI_CH4_NMI_UCX_Start_unpack(void *context, void *buffer, size_t count){

    MPI_Datatype *datatype = (MPI_Datatype*) context;
    MPID_Segment *segment_ptr;
    struct MPIDI_CH4_NMI_UCX_pack_state *state;
    MPI_Aint packsize;

    state = MPL_malloc(sizeof(struct MPIDI_CH4_NMI_UCX_pack_state)); 
    MPIR_Pack_size_impl(count, *datatype, &packsize);

    segment_ptr = MPID_Segment_alloc();

/* Todo: Add error handling */
    MPID_Segment_init(buffer, count, *datatype, segment_ptr, 1);
    state->packsize = packsize;
    state->segment_ptr = segment_ptr;
    return (void*) state;

}

static inline size_t  MPIDI_CH4_NMI_UCX_Packed_size(void *state) {

    struct MPIDI_CH4_NMI_UCX_pack_state *pack_state = (struct MPIDI_CH4_NMI_UCX_pack_state *) state;

    return (size_t) pack_state->packsize;
}

static inline size_t MPIDI_CH4_NMI_UCX_Pack(void *state, size_t offset, void *dest, size_t max_length){

    struct MPIDI_CH4_NMI_UCX_pack_state *pack_state = (struct MPIDI_CH4_NMI_UCX_pack_state *) state;
    MPI_Aint last = max_length;

    MPID_Segment_pack(pack_state->segment_ptr, offset, &last, dest);

    return (size_t) last;
}
static inline ucs_status_t MPIDI_CH4_NMI_UCX_Unpack(void *state, size_t offset, const void *src, size_t count){

    struct MPIDI_CH4_NMI_UCX_pack_state *pack_state = (struct MPIDI_CH4_NMI_UCX_pack_state *) state;

    MPID_Segment_unpack(pack_state->segment_ptr, offset, &count, (void*)src);

    return UCS_OK;
}

static inline  void MPIDI_CH4_NMI_UCX_Finish_pack(void *state){

    struct MPIDI_CH4_NMI_UCX_pack_state *pack_state = (struct MPIDI_CH4_NMI_UCX_pack_state *) state;
    MPID_Segment_free(pack_state->segment_ptr);
    MPL_free(pack_state);

}


static ucp_generic_dt_ops_t MPIDI_CH4_NMI_UCX_datatype_ops = {
    .start_pack   = MPIDI_CH4_NMI_UCX_Start_pack,
    .start_unpack = MPIDI_CH4_NMI_UCX_Start_unpack,
    .packed_size  = MPIDI_CH4_NMI_UCX_Packed_size,
    .pack         = MPIDI_CH4_NMI_UCX_Pack,
    .unpack       = MPIDI_CH4_NMI_UCX_Unpack,
    .finish       = MPIDI_CH4_NMI_UCX_Finish_pack
};


static inline void MPIDI_CH4_NM_datatype_destroy_hook(MPID_Datatype *datatype_p){

    int in_use;

    ucp_datatype_t ucp_datatype =  datatype_p->dev.netmod.ucx.ucp_datatype  ;

    if( datatype_p->dev.netmod.ucx.has_ucp == 1) {
        ucp_dt_destroy(ucp_datatype);
        MPIU_Object_release_ref(datatype_p, &in_use);
    }

}
static inline int MPIDI_CH4_NM_datatype_commit_hook(MPI_Datatype *datatype_p){
    int mpi_errno = MPI_SUCCESS;
    ucp_datatype_t ucp_datatype;
    ucs_status_t status;
    size_t size;
    MPID_Datatype *datatype_ptr;
    int is_contig;
    MPID_Datatype_get_ptr(*datatype_p, datatype_ptr);
    datatype_ptr->dev.netmod.ucx.has_ucp = 0;
    MPID_Datatype_is_contig(*datatype_p, &is_contig);
    if(is_contig)
        goto fn_exit;

    status = ucp_dt_create_generic(&MPIDI_CH4_NMI_UCX_datatype_ops,
                                         datatype_p, &ucp_datatype);
    MPIDI_CH4_NMI_UCX_CHK_STATUS(status, create_generic_datatype);
    datatype_ptr->dev.netmod.ucx.ucp_datatype = ucp_datatype;

    datatype_ptr->dev.netmod.ucx.has_ucp = 1;
    MPIU_Object_add_ref(datatype_ptr);
 fn_exit:
    return mpi_errno;
 fn_fail:
    goto fn_exit;
}
static inline int   MPIDI_CH4_NM_datatype_dup_hook(MPID_Datatype* datatype_ptr) {

    return  MPIDI_CH4_NM_datatype_commit_hook(&datatype_ptr->handle);
}

#endif /*UCX_DATATYPE_H_INCLUDED*/
