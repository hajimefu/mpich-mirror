[#] start of __file__
dnl MPICH_SUBCFG_AFTER=src/mpid/ch3/channels/nemesis

AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
    AM_COND_IF([BUILD_CH3_NEMESIS],[
        for net in $nemesis_networks ; do
            AS_CASE([$net],[ofi],[build_nemesis_netmod_ofi=yes])
        done
    ])
    AM_CONDITIONAL([BUILD_NEMESIS_NETMOD_OFI],[test "X$build_nemesis_netmod_ofi" = "Xyes"])
])dnl

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_COND_IF([BUILD_NEMESIS_NETMOD_OFI],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR ch3:nemesis:ofi])

    PAC_SET_HEADER_LIB_PATH(ofi)
    PAC_PUSH_FLAG(LIBS)
    PAC_CHECK_HEADER_LIB_FATAL(ofi, rdma/fabric.h, fabric, fi_getinfo)
    PAC_POP_FLAG(LIBS)

    if [test "$enable_sharedlibs" = "osx-gcc" ]; then
        PAC_APPEND_FLAG([-Wl,-rpath -Wl,${prefix}/lib], [LDFLAGS])
        PAC_APPEND_FLAG([-Wl,-rpath -Wl,${with_ofi}/lib], [LDFLAGS])
    else
        PAC_APPEND_FLAG([-Wl,-rpath -Wl,${prefix}/lib -Wl,--enable-new-dtags], [LDFLAGS])
        PAC_APPEND_FLAG([-Wl,-rpath -Wl,${with_ofi}/lib -Wl,--enable-new-dtags], [LDFLAGS])
    fi

    if [test "$do_static_fabric" = "true"]; then
       PAC_APPEND_FLAG([-L${with_ofi}/lib],[WRAPPER_STATIC_LDFLAGS])
       PAC_APPEND_FLAG([-lfabric],[WRAPPER_STATIC_LIBS])
    else
       PAC_APPEND_FLAG([-L${with_ofi}/lib -lfabric],[LIBS])
    fi
    PAC_APPEND_FLAG([-lstdc++ -ldl -lpthread],[LIBS])

    AC_DEFINE([ENABLE_COMM_OVERRIDES], 1, [define to add per-vc function pointers to override send and recv functions])
])dnl end AM_COND_IF(BUILD_NEMESIS_NETMOD_OFI,...)
])dnl end _BODY

[#] end of __file__
