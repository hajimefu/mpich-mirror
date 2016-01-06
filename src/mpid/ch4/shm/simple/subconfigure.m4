[#] start of __file__
dnl MPICH_SUBCFG_AFTER=src/mpid/ch4

AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
    AM_COND_IF([BUILD_CH4],[
        for shm in $ch4_shm ; do
            AS_CASE([$shm],[simple],[build_ch4_shm_simple=yes])
        done
    ])
    AM_CONDITIONAL([BUILD_CH4_SHM_SIMPLE],[test "X$build_ch4_shm_simple" = "Xyes"])
])dnl

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_COND_IF([BUILD_CH4_SHM_SIMPLE],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR ch4:shm:simple])
])dnl end AM_COND_IF(BUILD_CH4_SHM_SIMPLE,...)
])dnl end _BODY

PAC_ARG_SHARED_MEMORY

AC_ARG_ENABLE(nemesis-lock-free-queues,
              [--enable-nemesis-lock-free-queues - Use atomic instructions and lock-free queues for shared memory communication.  Lock-based queues will be used otherwise.  The default is enabled (lock-free).],
              , [enable_nemesis_lock_free_queues=yes])
if test "$enable_nemesis_lock_free_queues" = "yes" ; then
    AC_DEFINE(MPID_NEM_USE_LOCK_FREE_QUEUES, 1, [Define to enable lock-free communication queues])
fi

[#] end of __file__
