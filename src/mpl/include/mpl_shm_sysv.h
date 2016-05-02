/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/* vim: set ft=c.mpich : */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#ifndef MPL_SHM_SYSV_H_INCLUDED
#define MPL_SHM_SYSV_H_INCLUDED

typedef int MPLI_shm_lhnd_t;

#define MPL_SHM_MAX_INT_STR_LEN 20
#define MPL_SHM_SEG_KEY_LEN     MPL_SHM_MAX_INT_STR_LEN
#define MPLI_SHM_GHND_SZ        MPL_SHM_SEG_KEY_LEN
#define MPLI_SHM_LHND_INVALID   (-1)
#define MPLI_SHM_LHND_INIT_VAL  (-1)
#define MPL_SHM_SER_HND_SZ      MPLI_SHM_GHND_SZ

#define MPL_SHM_SEG_ALREADY_EXISTS EEXIST

/* Nothing to be done at close */
#define MPLI_shm_lhnd_close(hnd)    0

#endif /* ifndef MPL_SHM_SYSV_H_INCLUDED */
