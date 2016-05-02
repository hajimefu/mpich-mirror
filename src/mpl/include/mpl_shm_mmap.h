/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/* vim: set ft=c.mpich : */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#ifndef MPL_SHM_MMAP_H_INCLUDED
#define MPL_SHM_MMAP_H_INCLUDED

typedef intptr_t MPLI_shm_lhnd_t;

struct MPLI_shm_lghnd_t;
typedef struct MPLI_shm_lghnd_t* MPL_shm_hnd_t;

#define MPL_SHM_FNAME_LEN      50
#define MPLI_SHM_GHND_SZ       MPL_SHM_FNAME_LEN
#define MPLI_SHM_LHND_INVALID  (-1)
#define MPLI_SHM_LHND_INIT_VAL (-1)

#define MPLI_SHM_SEG_ALREADY_EXISTS EEXIST

/* Returns 0 on success, -1 on error */
int MPLI_shm_lhnd_close(MPL_shm_hnd_t hnd);

#endif /* ifndef MPL_SHM_MMAP_H_INCLUDED */
