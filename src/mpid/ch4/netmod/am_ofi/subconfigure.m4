[#] start of __file__
dnl MPICH_SUBCFG_AFTER=src/mpid/ch4

AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
    AM_COND_IF([BUILD_CH4],[
        for net in $ch4_netmods ; do
            AS_CASE([$net],[am_ofi],[build_ch4_netmod_am_ofi=yes])
	    if test $net = "am_ofi" ; then
	       if test "$build_ch4_locality_info" != "yes" ; then
	       	  AC_DEFINE(MPIDI_BUILD_CH4_LOCALITY_INFO, 1, [CH4 should build locality info])
		  build_ch4_locality_info="yes"
	       fi
	    fi
        done
    ])
    AM_CONDITIONAL([BUILD_CH4_NETMOD_AM_OFI],[test "X$build_ch4_netmod_am_ofi" = "Xyes"])
])dnl

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_COND_IF([BUILD_CH4_NETMOD_AM_OFI],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR ch4:am_ofi])

    PAC_SET_HEADER_LIB_PATH(ofi)
    PAC_PUSH_FLAG(LIBS)
    PAC_CHECK_HEADER_LIB_FATAL(ofi, rdma/fabric.h, fabric, fi_getinfo)
    PAC_APPEND_FLAG([-lfabric],[WRAPPER_LIBS])
    PAC_POP_FLAG(LIBS)
    PAC_APPEND_FLAG([-lstdc++],[WRAPPER_LIBS])

    AC_DEFINE([ENABLE_COMM_OVERRIDES], 1, [define to add per-vc function pointers to override send and recv functions])
])dnl end AM_COND_IF(BUILD_CH4_NETMOD_AM_OFI,...)
])dnl end _BODY

[#] end of __file__
