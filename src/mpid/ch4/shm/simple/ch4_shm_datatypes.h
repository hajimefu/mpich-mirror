/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2016 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#ifndef CH4_SHM_DATATYPES_H
#define CH4_SHM_DATATYPES_H

/* ************************************************************************** */
/* from mpid/ch3/channels/nemesis/include/mpid_nem_datatypes.h                */
/* ************************************************************************** */

#define MPIDI_CH4_SHMI_SIMPLE_OFFSETOF(struc, field) ((int)(&((struc *)0)->field))
#define MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN (64)
#define MPIDI_CH4_SHMI_SIMPLE_NUM_CELLS      (64)
#define MPIDI_CH4_SHMI_SIMPLE_CELL_LEN       (64*1024)

#if (SIZEOF_OPA_PTR_T > 8)
#  if (SIZEOF_OPA_PTR_T > 16)
#    error unexpected size for OPA_ptr_t
#  endif
#  define MPIDI_CH4_SHMI_SIMPLE_CELL_HEAD_LEN  16    /* We use this to keep elements 64-bit aligned */
#else /* (SIZEOF_OPA_PTR_T <= 8) */
#  define MPIDI_CH4_SHMI_SIMPLE_CELL_HEAD_LEN  8     /* We use this to keep elements 64-bit aligned */
#endif

/* cell header with matching info now is cache line */
#define MPIDI_CH4_SHMI_SIMPLE_CELL_PAYLOAD_LEN (MPIDI_CH4_SHMI_SIMPLE_CELL_LEN - MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN + MPIDI_CH4_SHMI_SIMPLE_MPICH_HEAD_LEN)

#define MPIDI_CH4_SHMI_SIMPLE_CALC_CELL_LEN(cellp) (MPIDI_CH4_SHMI_SIMPLE_CELL_HEAD_LEN + MPIDI_CH4_SHMI_SIMPLE_MPICH_HEAD_LEN + MPIDI_CH4_SHMI_SIMPLE_CELL_DLEN (cell))

#define MPIDI_CH4_SHMI_SIMPLE_ALIGNED(addr, bytes) ((((unsigned long)addr) & (((unsigned long)bytes)-1)) == 0)

#define MPIDI_CH4_SHMI_SIMPLE_PKT_UNKNOWN     0
#define MPIDI_CH4_SHMI_SIMPLE_PKT_MPICH      1
#define MPIDI_CH4_SHMI_SIMPLE_PKT_MPICH_HEAD 2

#define MPIDI_CH4_SHMI_SIMPLE_FBOX_SOURCE(cell) (MPIDI_CH4_SHMI_SIMPLE_mem_region.local_procs[(cell)->pkt.mpich.source])
#define MPIDI_CH4_SHMI_SIMPLE_CELL_SOURCE(cell) ((cell)->pkt.mpich.source)
#define MPIDI_CH4_SHMI_SIMPLE_CELL_DEST(cell)   ((cell)->pkt.mpich.dest)
#define MPIDI_CH4_SHMI_SIMPLE_CELL_DLEN(cell)   ((cell)->pkt.mpich.datalen)
#define MPIDI_CH4_SHMI_SIMPLE_CELL_SEQN(cell)   ((cell)->pkt.mpich.seqno)

#define MPIDI_CH4_SHMI_SIMPLE_MPICH_HEAD_LEN sizeof(MPIDI_CH4_SHMI_SIMPLE_Pkt_header_t)
#define MPIDI_CH4_SHMI_SIMPLE_DATA_LEN (MPIDI_CH4_SHMI_SIMPLE_CELL_PAYLOAD_LEN - MPIDI_CH4_SHMI_SIMPLE_MPICH_HEAD_LEN)

#define MPIDI_CH4_SHMI_SIMPLE_PKT_HEADER_FIELDS          \
    int source;                             \
    int dest;                               \
    MPIU_Pint datalen;                      \
    unsigned short seqno;                   \
    unsigned short type;        /* currently used only with checkpointing */

typedef struct MPIDI_CH4_SHMI_SIMPLE_Pkt_header {
    MPIDI_CH4_SHMI_SIMPLE_PKT_HEADER_FIELDS;
} MPIDI_CH4_SHMI_SIMPLE_Pkt_header_t;

typedef struct MPIDI_CH4_SHMI_SIMPLE_Pkt_mpich {
    MPIDI_CH4_SHMI_SIMPLE_PKT_HEADER_FIELDS;
    union {
        char payload[MPIDI_CH4_SHMI_SIMPLE_DATA_LEN];
        double dummy;           /* align paylod to double */
    } p;
} MPIDI_CH4_SHMI_SIMPLE_Pkt_mpich_t;

typedef union {
    MPIDI_CH4_SHMI_SIMPLE_Pkt_header_t header;
    MPIDI_CH4_SHMI_SIMPLE_Pkt_mpich_t mpich;
} MPIDI_CH4_SHMI_SIMPLE_Pkt_t;

/* Nemesis cells which are to be used in shared memory need to use
 * "relative pointers" because the absolute pointers to a cell from
 * different processes may be different.  Relative pointers are
 * offsets from the beginning of the mmapped region where they live.
 * We use different types for relative and absolute pointers to help
 * catch errors.  Use MPIDI_CH4_SHMI_SIMPLE_REL_TO_ABS and MPIDI_CH4_SHMI_SIMPLE_ABS_TO_REL to
 * convert between relative and absolute pointers. */

/* This should always be exactly the size of a pointer */
typedef struct MPIDI_CH4_SHMI_SIMPLE_Cell_rel_ptr {
    OPA_ptr_t p;
} MPIDI_CH4_SHMI_SIMPLE_Cell_rel_ptr_t;

/* MPIDI_CH4_SHMI_SIMPLE_Cell and MPIDI_CH4_SHMI_SIMPLE_Abs_cell must be kept in sync so that we
 * can cast between them.  MPIDI_CH4_SHMI_SIMPLE_Abs_cell should only be used when
 * a cell is enqueued on a queue local to a single process (e.g., a
 * queue in a network module) where relative pointers are not
 * needed. */

typedef struct MPIDI_CH4_SHMI_SIMPLE_Cell {
    MPIDI_CH4_SHMI_SIMPLE_Cell_rel_ptr_t next;
#if (MPIDI_CH4_SHMI_SIMPLE_CELL_HEAD_LEN > SIZEOF_OPA_PTR_T)
    char padding[MPIDI_CH4_SHMI_SIMPLE_CELL_HEAD_LEN - sizeof(MPIDI_CH4_SHMI_SIMPLE_Cell_rel_ptr_t)];
#endif
    int my_rank;
    int rank;
    int tag;
    int context_id;
    MPID_Request* pending;
#if MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN != 0
    char padding[MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN - MPIDI_CH4_SHMI_SIMPLE_CELL_HEAD_LEN - MPIDI_CH4_SHMI_SIMPLE_MPICH_HEAD_LEN - 4 * sizeof(int) - sizeof(MPID_Request*)]; /* should be 64-16-16-16-8 = 8 */
#endif
    volatile MPIDI_CH4_SHMI_SIMPLE_Pkt_t pkt;
} MPIDI_CH4_SHMI_SIMPLE_Cell_t;
typedef MPIDI_CH4_SHMI_SIMPLE_Cell_t *MPIDI_CH4_SHMI_SIMPLE_Cell_ptr_t;

typedef struct MPIDI_CH4_SHMI_SIMPLE_Abs_cell {
    struct MPIDI_CH4_SHMI_SIMPLE_Abs_cell *next;
#if (MPIDI_CH4_SHMI_SIMPLE_CELL_HEAD_LEN > SIZEOF_VOID_P)
    char padding[MPIDI_CH4_SHMI_SIMPLE_CELL_HEAD_LEN - sizeof(struct MPIDI_CH4_SHMI_SIMPLE_Abs_cell *)];
#endif
    volatile MPIDI_CH4_SHMI_SIMPLE_Pkt_t pkt;
} MPIDI_CH4_SHMI_SIMPLE_Abs_cell_t;
typedef MPIDI_CH4_SHMI_SIMPLE_Abs_cell_t *MPIDI_CH4_SHMI_SIMPLE_Abs_cell_ptr_t;

#define MPIDI_CH4_SHMI_SIMPLE_CELL_TO_PACKET(cellp) (&(cellp)->pkt)
#define MPIDI_CH4_SHMI_SIMPLE_PACKET_TO_CELL(packetp) \
    ((MPIDI_CH4_SHMI_SIMPLE_Cell_ptr_t) ((char*)(packetp) - (char *)MPIDI_CH4_SHMI_SIMPLE_CELL_TO_PACKET((MPIDI_CH4_SHMI_SIMPLE_Cell_ptr_t)0)))
#define MPIDI_CH4_SHMI_SIMPLE_MIN_PACKET_LEN (sizeof (MPIDI_CH4_SHMI_SIMPLE_Pkt_header_t))
#define MPIDI_CH4_SHMI_SIMPLE_MAX_PACKET_LEN (sizeof (MPIDI_CH4_SHMI_SIMPLE_Pkt_t))
#define MPIDI_CH4_SHMI_SIMPLE_PACKET_LEN(pkt) ((pkt)->mpich.datalen + MPIDI_CH4_SHMI_SIMPLE_MPICH_HEAD_LEN)

#define MPIDI_CH4_SHMI_SIMPLE_OPT_LOAD     16
#define MPIDI_CH4_SHMI_SIMPLE_OPT_SIZE     ((sizeof(MPIDI_CH3_Pkt_t)) + (MPIDI_CH4_SHMI_SIMPLE_OPT_LOAD))
#define MPIDI_CH4_SHMI_SIMPLE_OPT_HEAD_LEN ((MPIDI_CH4_SHMI_SIMPLE_MPICH_HEAD_LEN) + (MPIDI_CH4_SHMI_SIMPLE_OPT_SIZE))

#define MPIDI_CH4_SHMI_SIMPLE_PACKET_OPT_LEN(pkt) \
    (((pkt)->mpich.datalen < MPIDI_CH4_SHMI_SIMPLE_OPT_SIZE) ? (MPIDI_CH4_SHMI_SIMPLE_OPT_HEAD_LEN) : (MPIDI_CH4_SHMI_SIMPLE_PACKET_LEN(pkt)))

#define MPIDI_CH4_SHMI_SIMPLE_PACKET_PAYLOAD(pkt) ((pkt)->mpich.payload)

typedef struct MPIDI_CH4_SHMI_SIMPLE_Queue {
    MPIDI_CH4_SHMI_SIMPLE_Cell_rel_ptr_t head;
    MPIDI_CH4_SHMI_SIMPLE_Cell_rel_ptr_t tail;
#if (MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN > (2 * SIZEOF_OPA_PTR_T))
    char padding1[MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN - 2 * sizeof(MPIDI_CH4_SHMI_SIMPLE_Cell_rel_ptr_t)];
#endif
    MPIDI_CH4_SHMI_SIMPLE_Cell_rel_ptr_t my_head;
#if (MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN > SIZEOF_OPA_PTR_T)
    char padding2[MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN - sizeof(MPIDI_CH4_SHMI_SIMPLE_Cell_rel_ptr_t)];
#endif
#if !defined(MPIDI_CH4_SHMI_SIMPLE_USE_LOCK_FREE_QUEUES)
    /* see FIXME in mpid_nem_queue.h */
#define MPIDI_CH4_SHMI_SIMPLE_Queue_mutex_t MPID_Thread_mutex_t
    MPIDI_CH4_SHMI_SIMPLE_Queue_mutex_t lock;
    char padding3[MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN - sizeof(MPID_Thread_mutex_t)];
#endif
} MPIDI_CH4_SHMI_SIMPLE_Queue_t, *MPIDI_CH4_SHMI_SIMPLE_Queue_ptr_t;

/* Fast Boxes*/
typedef union {
    OPA_int_t value;
#if MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN != 0
    char padding[MPIDI_CH4_SHMI_SIMPLE_CACHE_LINE_LEN];
#endif
} MPIDI_CH4_SHMI_SIMPLE_Opt_volint_t;

typedef struct MPIDI_CH4_SHMI_SIMPLE_Fbox_common {
    MPIDI_CH4_SHMI_SIMPLE_Opt_volint_t flag;
} MPIDI_CH4_SHMI_SIMPLE_Fbox_common_t, *MPIDI_CH4_SHMI_SIMPLE_Fbox_common_ptr_t;

typedef struct MPIDI_CH4_SHMI_SIMPLE_Fbox_mpich {
    MPIDI_CH4_SHMI_SIMPLE_Opt_volint_t flag;
    MPIDI_CH4_SHMI_SIMPLE_Cell_t cell;
} MPIDI_CH4_SHMI_SIMPLE_Fbox_mpich_t;

#define MPIDI_CH4_SHMI_SIMPLE_FBOX_DATALEN MPIDI_CH4_SHMI_SIMPLE_DATA_LEN

typedef union {
    MPIDI_CH4_SHMI_SIMPLE_Fbox_common_t common;
    MPIDI_CH4_SHMI_SIMPLE_Fbox_mpich_t mpich;
} MPIDI_CH4_SHMI_SIMPLE_Fastbox_t;

typedef struct MPIDI_CH4_SHMI_SIMPLE_Fbox_arrays {
    MPIDI_CH4_SHMI_SIMPLE_Fastbox_t **in;
    MPIDI_CH4_SHMI_SIMPLE_Fastbox_t **out;
} MPIDI_CH4_SHMI_SIMPLE_Fbox_arrays_t;

#endif /* ifndef CH4_SHM_DATATYPES_H */
