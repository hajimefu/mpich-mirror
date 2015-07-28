/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <map>
#include <queue>
#include <new>
#include <mpidimpl.h>
#include "impl.h"

EXTERN_C_BEGIN

#undef FUNCNAME
#define FUNCNAME MPIDI_VCRT_Release
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_VCRT_Release(struct MPIDI_VCRT *vcrt)
{
    int count;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_VCRT_RELEASE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_VCRT_RELEASE);
    MPIU_Object_release_ref(vcrt, &count);

    if(count == 0)
        MPIU_Free(vcrt);

    MPIDI_FUNC_EXIT(MPID_STATE_NETMOD_OFI_VCRT_RELEASE);
    return MPI_SUCCESS;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_VCRT_Create
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_VCRT_Create(int size, struct MPIDI_VCRT **vcrt_ptr)
{
    struct MPIDI_VCRT *vcrt;
    int i,mpi_errno;
    MPIDI_STATE_DECL(MPID_STATE_NETMOD_OFI_VCRT_CREATE);
    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_VCRT_CREATE);

    vcrt = (struct MPIDI_VCRT *)MPIU_Malloc(sizeof(struct MPIDI_VCRT) +
					    size*sizeof(MPIDI_VCR));

    if(vcrt != NULL) {
	MPIU_Object_set_ref(vcrt, 1);
	vcrt->size = size;
	*vcrt_ptr = vcrt;

	for(i=0; i<size; i++)
	    vcrt->vcr_table[i].addr_idx = i;

	mpi_errno = MPI_SUCCESS;
    } else
	mpi_errno = MPIR_ERR_MEMALLOCFAILED;

    MPIDI_FUNC_ENTER(MPID_STATE_NETMOD_OFI_VCRT_EXIT);
    return mpi_errno;
}


typedef std::map<uint64_t,void *>  uint64_map;
void MPIDI_Map_create(void **_map)
{
    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_ThreadInfo.global_mutex);
    *_map = (void *)MPIU_Malloc(sizeof(uint64_map));
    new(*_map) uint64_map();
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_ThreadInfo.global_mutex);
}

void MPIDI_Map_destroy(void *_map)
{
    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_ThreadInfo.global_mutex);
    uint64_map *m = (uint64_map *)_map;
    m->~uint64_map();
    MPIU_Free(_map);
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_ThreadInfo.global_mutex);
}

void MPIDI_Map_set(void     *_map,
                   uint64_t  id,
                   void     *val)
{
    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_ThreadInfo.global_mutex);
    uint64_map *m = (uint64_map *)_map;
    (*m)[id] = val;
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_ThreadInfo.global_mutex);
}

void MPIDI_Map_erase(void     *_map,
                     uint64_t  id)
{
    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_ThreadInfo.global_mutex);
    uint64_map *m = (uint64_map *)_map;
    m->erase(id);
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_ThreadInfo.global_mutex);
}

void *MPIDI_Map_lookup(void     *_map,
                       uint64_t  id)
{
    uint64_map *m = (uint64_map *)_map;
    void       *rc;
    MPID_THREAD_CS_ENTER(GLOBAL, MPIR_ThreadInfo.global_mutex);
    if(m->find(id) == m->end())
	rc = MPIDI_MAP_NOT_FOUND;
    else
	rc = (*m)[id];
    MPID_THREAD_CS_EXIT(GLOBAL, MPIR_ThreadInfo.global_mutex);
    return rc;
}

void MPIDI_build_nodemap(uint32_t       *in_nodeids,
                         MPID_Node_id_t *out_nodemap,
                         int             sz,
                         MPID_Node_id_t *sz_out)
{
    typedef std::multimap<uint32_t,uint32_t> nodemap;
    typedef std::pair<uint32_t,uint32_t>     nodepair;
    typedef nodemap::iterator                mapiter;
    typedef std::pair<mapiter, mapiter>      mappair;

    nodemap m;
    int i;

    for(i=0; i<sz; i++)
	m.insert(nodepair(in_nodeids[i],i));

    for(i=0; i<sz; i++)
	out_nodemap[i] = 0xFFFF;

    MPID_Node_id_t node_id = 0;

    for(mapiter it = m.begin(), end=m.end();
	it != end;
	it = m.upper_bound(it->first)) {
	mappair p;
	p = m.equal_range(it->first);

	for(mapiter it2 = p.first;
	    it2 != p.second;
	    it2++)
	    out_nodemap[it2->second] = node_id;

	node_id++;
	MPIU_Assert(node_id != 0xFFFF);
    }

    *sz_out = node_id;
}

/* MPI Datatype Processing for RMA */
#define isS_INT(x) ( (x)==MPI_INTEGER ||                                \
		     (x) == MPI_INT32_T || (x) == MPI_INTEGER4 ||       \
                     (x) == MPI_INT)
#define isUS_INT(x) ( (x) == MPI_UINT32_T || (x) == MPI_UNSIGNED)
#define isS_SHORT(x) ( (x) == MPI_SHORT || (x) == MPI_INT16_T ||        \
                       (x) == MPI_INTEGER2)
#define isUS_SHORT(x) ( (x) == MPI_UNSIGNED_SHORT || (x) == MPI_UINT16_T)
#define isS_CHAR(x) ( (x) == MPI_SIGNED_CHAR || (x) == MPI_INT8_T ||    \
                      (x) == MPI_INTEGER1 || (x) == MPI_CHAR)
#define isUS_CHAR(x) ( (x) == MPI_BYTE ||                               \
                       (x) == MPI_UNSIGNED_CHAR || (x) == MPI_UINT8_T )
#define isS_LONG(x) ( (x) == MPI_LONG || (x) == MPI_AINT )
#define isUS_LONG(x) ( (x) == MPI_UNSIGNED_LONG )
#define isS_LONG_LONG(x) ( (x) == MPI_INT64_T || (x) == MPI_OFFSET ||   \
			   (x) == MPI_INTEGER8 || (x) == MPI_LONG_LONG || \
                           (x) == MPI_LONG_LONG_INT || (x) == MPI_COUNT )
#define isUS_LONG_LONG(x) ( (x) == MPI_UINT64_T || (x) == MPI_UNSIGNED_LONG_LONG )
#define isFLOAT(x) ( (x) == MPI_FLOAT || (x) == MPI_REAL)
#define isDOUBLE(x) ( (x) == MPI_DOUBLE || (x) == MPI_DOUBLE_PRECISION)
#define isLONG_DOUBLE(x) ( (x) == MPI_LONG_DOUBLE )
#define isLOC_TYPE(x) ( (x) == MPI_2REAL || (x) == MPI_2DOUBLE_PRECISION || \
			(x) == MPI_2INTEGER || (x) == MPI_FLOAT_INT ||  \
			(x) == MPI_DOUBLE_INT || (x) == MPI_LONG_INT || \
			(x) == MPI_2INT || (x) == MPI_SHORT_INT ||      \
                        (x) == MPI_LONG_DOUBLE_INT )
#define isBOOL(x) ( (x) == MPI_C_BOOL )
#define isLOGICAL(x) ( (x) == MPI_LOGICAL )
#define isSINGLE_COMPLEX(x) ( (x) == MPI_COMPLEX || (x) == MPI_C_FLOAT_COMPLEX)
#define isDOUBLE_COMPLEX(x) ( (x) == MPI_DOUBLE_COMPLEX || (x) == MPI_COMPLEX8 || \
                              (x) == MPI_C_DOUBLE_COMPLEX)

EXTERN_C_END
