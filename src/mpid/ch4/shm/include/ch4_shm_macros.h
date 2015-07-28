#define __INLINE_ATTRIBUTE__ __attribute__((always_inline))
#define __SHM_INLINE__ __INLINE_ATTRIBUTE__ static inline
/* Macros from:                                                           */
/* http://stackoverflow.com/questions/10506893/pushing-the-c-preprocessor */
/* Ok to use this?                                                        */
#define EAT(...)
#define REM(...) __VA_ARGS__
#define STRIP(x) EAT x
#define PAIR(x)  REM x
#define NARGS_SEQ(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,N,...) N
#define NARGS(...) NARGS_SEQ(__VA_ARGS__,13,12,11,10,9,8,7,6,5,4,3,2,1)
#define PRIMITIVE_CAT(x, y) x ## y
#define CAT(x, y) PRIMITIVE_CAT(x, y)
#define APPLY(macro, ...) CAT(APPLY_, NARGS(__VA_ARGS__))(macro, __VA_ARGS__)
#define APPLY_1(m, x1) m(x1)
#define APPLY_2(m, x1, x2) m(x1), m(x2)
#define APPLY_3(m, x1, x2, x3) m(x1), m(x2), m(x3)
#define APPLY_4(m, x1, x2, x3, x4) m(x1), m(x2), m(x3), m(x4)
#define APPLY_5(m, x1, x2, x3, x4, x5) m(x1), m(x2), m(x3), m(x4), m(x5)
#define APPLY_6(m, x1, x2, x3, x4, x5, x6) m(x1), m(x2), m(x3), m(x4), m(x5), m(x6)
#define APPLY_7(m, x1, x2, x3, x4, x5, x6, x7) m(x1), m(x2), m(x3), m(x4), m(x5), m(x6), m(x7)
#define APPLY_8(m, x1, x2, x3, x4, x5, x6, x7, x8) m(x1), m(x2), m(x3), m(x4), m(x5), m(x6), m(x7), m(x8)
#define APPLY_9(m, x1, x2, x3, x4, x5, x6, x7, x8, x9) m(x1), m(x2), m(x3), m(x4), m(x5), m(x6), m(x7), m(x8), m(x9)
#define APPLY_10(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) m(x1), m(x2), m(x3), m(x4), m(x5), m(x6), m(x7), m(x8), m(x9), m(x10)
#define APPLY_11(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11) m(x1), m(x2), m(x3), m(x4), m(x5), m(x6), m(x7), m(x8), m(x9), m(x10), m(x11)
#define APPLY_12(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12) m(x1), m(x2), m(x3), m(x4), m(x5), m(x6), m(x7), m(x8), m(x9), m(x10), m(x11), m(x12)
#define APPLY_13(m, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13) m(x1), m(x2), m(x3), m(x4), m(x5), m(x6), m(x7), m(x8), m(x9), m(x10), m(x11), m(x12), m(x13)

#ifdef USE_SHM_TYPEDEFS
#define MPIDI_SHM_API(rc,fcnname,...)                              \
  typedef rc (*MPIDI_shm_##fcnname##_t)(APPLY(PAIR,__VA_ARGS__))
#endif

#ifdef USE_SHM_PROTOTYPES
#define MPIDI_SHM_API(rc,fcnname,...)                                \
  __SHM_INLINE__ rc MPIDI_shm_##fcnname(APPLY(PAIR,__VA_ARGS__))  \
  __INLINE_ATTRIBUTE__
#endif

#ifdef USE_SHM_INLINES
#define MPIDI_SHM_API(rc,fcnname,...)                                \
  __SHM_INLINE__ rc MPIDI_shm_##fcnname(APPLY(PAIR,__VA_ARGS__)) { \
    return MPIDI_shm_funcs.fcnname(APPLY(STRIP, __VA_ARGS__));       \
  }
#endif

#ifdef USE_SHM_TYPEDEFS
#define MPIDI_SHM_API_NATIVE(rc,fcnname,...)                       \
  typedef rc (*MPIDI_shm_##fcnname##_t)(APPLY(PAIR,__VA_ARGS__))
#endif

#ifdef USE_SHM_PROTOTYPES
#define MPIDI_SHM_API_NATIVE(rc,fcnname,...)                         \
  __SHM_INLINE__ rc MPIDI_shm_##fcnname(APPLY(PAIR,__VA_ARGS__))  \
  __INLINE_ATTRIBUTE__
#endif

#ifdef USE_SHM_INLINES
#define MPIDI_SHM_API_NATIVE(rc,fcnname,...)                         \
  __SHM_INLINE__ rc MPIDI_shm_##fcnname(APPLY(PAIR,__VA_ARGS__)) { \
    return MPIDI_shm_native_funcs.fcnname(APPLY(STRIP, __VA_ARGS__)); \
  }
#endif
