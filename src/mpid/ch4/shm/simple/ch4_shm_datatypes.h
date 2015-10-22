/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/* ************************************************************************** */
/* from mpid/ch3/channels/nemesis/include/mpid_nem_datatypes.h                */
/* ************************************************************************** */

#define MPID_NEM_OFFSETOF(struc, field) ((int)(&((struc *)0)->field))
#define MPID_NEM_CACHE_LINE_LEN (64)
#define MPID_NEM_NUM_CELLS      (64)
#define MPID_NEM_CELL_LEN       (64*1024)

#if (SIZEOF_OPA_PTR_T > 8)
#  if (SIZEOF_OPA_PTR_T > 16)
#    error unexpected size for OPA_ptr_t
#  endif
#  define MPID_NEM_CELL_HEAD_LEN  16    /* We use this to keep elements 64-bit aligned */
#else /* (SIZEOF_OPA_PTR_T <= 8) */
#  define MPID_NEM_CELL_HEAD_LEN  8     /* We use this to keep elements 64-bit aligned */
#endif

/* cell header with matching info now is cache line */
#define MPID_NEM_CELL_PAYLOAD_LEN (MPID_NEM_CELL_LEN - MPID_NEM_CACHE_LINE_LEN + MPID_NEM_MPICH_HEAD_LEN)

#define MPID_NEM_CALC_CELL_LEN(cellp) (MPID_NEM_CELL_HEAD_LEN + MPID_NEM_MPICH_HEAD_LEN + MPID_NEM_CELL_DLEN (cell))

#define MPID_NEM_ALIGNED(addr, bytes) ((((unsigned long)addr) & (((unsigned long)bytes)-1)) == 0)

#define MPID_NEM_PKT_UNKNOWN     0
#define MPID_NEM_PKT_MPICH      1
#define MPID_NEM_PKT_MPICH_HEAD 2

#define MPID_NEM_FBOX_SOURCE(cell) (MPID_nem_mem_region.local_procs[(cell)->pkt.mpich.source])
#define MPID_NEM_CELL_SOURCE(cell) ((cell)->pkt.mpich.source)
#define MPID_NEM_CELL_DEST(cell)   ((cell)->pkt.mpich.dest)
#define MPID_NEM_CELL_DLEN(cell)   ((cell)->pkt.mpich.datalen)
#define MPID_NEM_CELL_SEQN(cell)   ((cell)->pkt.mpich.seqno)

#define MPID_NEM_MPICH_HEAD_LEN sizeof(MPID_nem_pkt_header_t)
#define MPID_NEM_MPICH_DATA_LEN (MPID_NEM_CELL_PAYLOAD_LEN - MPID_NEM_MPICH_HEAD_LEN)

#define MPID_NEM_PKT_HEADER_FIELDS          \
    int source;                             \
    int dest;                               \
    MPIU_Pint datalen;                      \
    unsigned short seqno;                   \
    unsigned short type;        /* currently used only with checkpointing */

typedef struct MPID_nem_pkt_header {
    MPID_NEM_PKT_HEADER_FIELDS;
} MPID_nem_pkt_header_t;

typedef struct MPID_nem_pkt_mpich {
    MPID_NEM_PKT_HEADER_FIELDS;
    union {
        char payload[MPID_NEM_MPICH_DATA_LEN];
        double dummy;           /* align paylod to double */
    } p;
} MPID_nem_pkt_mpich_t;

typedef union {
    MPID_nem_pkt_header_t header;
    MPID_nem_pkt_mpich_t mpich;
} MPID_nem_pkt_t;

/* Nemesis cells which are to be used in shared memory need to use
 * "relative pointers" because the absolute pointers to a cell from
 * different processes may be different.  Relative pointers are
 * offsets from the beginning of the mmapped region where they live.
 * We use different types for relative and absolute pointers to help
 * catch errors.  Use MPID_NEM_REL_TO_ABS and MPID_NEM_ABS_TO_REL to
 * convert between relative and absolute pointers. */

/* This should always be exactly the size of a pointer */
typedef struct MPID_nem_cell_rel_ptr {
    OPA_ptr_t p;
} MPID_nem_cell_rel_ptr_t;

/* MPID_nem_cell and MPID_nem_abs_cell must be kept in sync so that we
 * can cast between them.  MPID_nem_abs_cell should only be used when
 * a cell is enqueued on a queue local to a single process (e.g., a
 * queue in a network module) where relative pointers are not
 * needed. */

typedef struct MPID_nem_cell {
    MPID_nem_cell_rel_ptr_t next;
#if (MPID_NEM_CELL_HEAD_LEN > SIZEOF_OPA_PTR_T)
    char padding[MPID_NEM_CELL_HEAD_LEN - sizeof(MPID_nem_cell_rel_ptr_t)];
#endif
    int my_rank;
    int rank;
    int tag;
    int context_id;
    MPID_Request* pending;
#if MPID_NEM_CACHE_LINE_LEN != 0
    char padding[MPID_NEM_CACHE_LINE_LEN - MPID_NEM_CELL_HEAD_LEN - MPID_NEM_MPICH_HEAD_LEN - 4 * sizeof(int) - sizeof(MPID_Request*)]; /* should be 64-16-16-16-8 = 8 */
#endif
    volatile MPID_nem_pkt_t pkt;
} MPID_nem_cell_t;
typedef MPID_nem_cell_t *MPID_nem_cell_ptr_t;

typedef struct MPID_nem_abs_cell {
    struct MPID_nem_abs_cell *next;
#if (MPID_NEM_CELL_HEAD_LEN > SIZEOF_VOID_P)
    char padding[MPID_NEM_CELL_HEAD_LEN - sizeof(struct MPID_nem_abs_cell *)];
#endif
    volatile MPID_nem_pkt_t pkt;
} MPID_nem_abs_cell_t;
typedef MPID_nem_abs_cell_t *MPID_nem_abs_cell_ptr_t;

#define MPID_NEM_CELL_TO_PACKET(cellp) (&(cellp)->pkt)
#define MPID_NEM_PACKET_TO_CELL(packetp) \
    ((MPID_nem_cell_ptr_t) ((char*)(packetp) - (char *)MPID_NEM_CELL_TO_PACKET((MPID_nem_cell_ptr_t)0)))
#define MPID_NEM_MIN_PACKET_LEN (sizeof (MPID_nem_pkt_header_t))
#define MPID_NEM_MAX_PACKET_LEN (sizeof (MPID_nem_pkt_t))
#define MPID_NEM_PACKET_LEN(pkt) ((pkt)->mpich.datalen + MPID_NEM_MPICH_HEAD_LEN)

#define MPID_NEM_OPT_LOAD     16
#define MPID_NEM_OPT_SIZE     ((sizeof(MPIDI_CH3_Pkt_t)) + (MPID_NEM_OPT_LOAD))
#define MPID_NEM_OPT_HEAD_LEN ((MPID_NEM_MPICH_HEAD_LEN) + (MPID_NEM_OPT_SIZE))

#define MPID_NEM_PACKET_OPT_LEN(pkt) \
    (((pkt)->mpich.datalen < MPID_NEM_OPT_SIZE) ? (MPID_NEM_OPT_HEAD_LEN) : (MPID_NEM_PACKET_LEN(pkt)))

#define MPID_NEM_PACKET_PAYLOAD(pkt) ((pkt)->mpich.payload)

typedef struct MPID_nem_queue {
    MPID_nem_cell_rel_ptr_t head;
    MPID_nem_cell_rel_ptr_t tail;
#if (MPID_NEM_CACHE_LINE_LEN > (2 * SIZEOF_OPA_PTR_T))
    char padding1[MPID_NEM_CACHE_LINE_LEN - 2 * sizeof(MPID_nem_cell_rel_ptr_t)];
#endif
    MPID_nem_cell_rel_ptr_t my_head;
#if (MPID_NEM_CACHE_LINE_LEN > SIZEOF_OPA_PTR_T)
    char padding2[MPID_NEM_CACHE_LINE_LEN - sizeof(MPID_nem_cell_rel_ptr_t)];
#endif
#if !defined(MPID_NEM_USE_LOCK_FREE_QUEUES)
    /* see FIXME in mpid_nem_queue.h */
#define MPID_nem_queue_mutex_t MPID_Thread_mutex_t
    MPID_nem_queue_mutex_t lock;
    char padding3[MPID_NEM_CACHE_LINE_LEN - sizeof(MPID_Thread_mutex_t)];
#endif
} MPID_nem_queue_t, *MPID_nem_queue_ptr_t;

/* Fast Boxes*/
typedef union {
    OPA_int_t value;
#if MPID_NEM_CACHE_LINE_LEN != 0
    char padding[MPID_NEM_CACHE_LINE_LEN];
#endif
} MPID_nem_opt_volint_t;

typedef struct MPID_nem_fbox_common {
    MPID_nem_opt_volint_t flag;
} MPID_nem_fbox_common_t, *MPID_nem_fbox_common_ptr_t;

typedef struct MPID_nem_fbox_mpich {
    MPID_nem_opt_volint_t flag;
    MPID_nem_cell_t cell;
} MPID_nem_fbox_mpich_t;

#define MPID_NEM_FBOX_DATALEN MPID_NEM_MPICH_DATA_LEN

typedef union {
    MPID_nem_fbox_common_t common;
    MPID_nem_fbox_mpich_t mpich;
} MPID_nem_fastbox_t;

typedef struct MPID_nem_fbox_arrays {
    MPID_nem_fastbox_t **in;
    MPID_nem_fastbox_t **out;
} MPID_nem_fbox_arrays_t;
