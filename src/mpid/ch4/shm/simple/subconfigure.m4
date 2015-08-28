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

[#] end of __file__
