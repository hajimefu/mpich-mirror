[#] start of __file__
dnl MPICH_SUBCFG_AFTER=src/mpid/ch4

AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
    AM_COND_IF([BUILD_CH4],[
        for net in $ch4_netmods ; do
            AS_CASE([$net],[ofi],[build_ch4_netmod_ofi=yes])
	    if test $net == "ofi" ; then
	       AC_DEFINE(HAVE_CH4_NETMOD_OFI,1,[OFI netmod is built])
	       if test "$build_ch4_locality_info" != "yes" ; then
	       	  AC_DEFINE(MPIDI_BUILD_CH4_LOCALITY_INFO, 1, [CH4 should build locality info])
		  build_ch4_locality_info="yes"
	       fi
	    fi
        done
    ])
    AM_CONDITIONAL([BUILD_CH4_NETMOD_OFI],[test "X$build_ch4_netmod_ofi" = "Xyes"])
])dnl

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_COND_IF([BUILD_CH4_NETMOD_OFI],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR ch4:ofi])

    PAC_SET_HEADER_LIB_PATH(ofi)
    PAC_PUSH_FLAG(LIBS)
    PAC_CHECK_HEADER_LIB_FATAL(ofi, rdma/fabric.h, fabric, fi_getinfo)
    PAC_APPEND_FLAG([-lfabric],[WRAPPER_LIBS])
    PAC_POP_FLAG(LIBS)
    PAC_APPEND_FLAG([-lstdc++],[WRAPPER_LIBS])
])dnl end AM_COND_IF(BUILD_CH4_NETMOD_OFI,...)
])dnl end _BODY

[#] end of __file__
