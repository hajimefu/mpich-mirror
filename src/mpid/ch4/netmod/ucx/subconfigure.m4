[#] start of __file__
dnl MPICH_SUBCFG_AFTER=src/mpid/ch4

AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
    AM_COND_IF([BUILD_CH4],[
        for net in $ch4_netmods ; do
            AS_CASE([$net],[ucx],[build_ch4_netmod_ucx=yes])
	    if test $net = "ucx" ; then
	       AC_DEFINE(HAVE_CH4_NETMOD_UCX,1,[UCX netmod is built])
	       if test "$build_ch4_locality_info" != "yes" ; then
	          AC_DEFINE(MPIDI_BUILD_CH4_LOCALITY_INFO, 1, [CH4 should build locality info])
		  build_ch4_locality_info="yes"
	       fi
	    fi
        done

dnl       AC_ARG_WITH(ch4-netmod-ofi-args,
dnl       [  --with-ch4-netmod-ofi-args=arg1:arg2:arg3
dnl       CH4 OFI netmod arguments:
dnl               scalable-endpoints - Use OFI scalable endpoint mode
dnl               av-table           - Use OFI AV table (logical addressing mode).  Default is av-map mode
dnl               mr-offset          - USE OFI memory region offset mode
dnl               direct-provider    - USE OFI FI_DIRECT to compile in a single OFI direct provider
dnl               static-fabric      - Statically link libfabric
dnl               ],
dnl               [ofi_netmod_args=$withval],
dnl               [ofi_netmod_args=])

dnl Parse the device arguments
dnl    SAVE_IFS=$IFS
dnl    IFS=':'
dnl    args_array=$ofi_netmod_args
dnl    do_scalable_endpoints=false
dnl    do_direct_provider=false
dnl    do_mr_offset=false
dnl    do_av_table=false
dnl    do_static_fabric=false
dnl    echo "Parsing Arguments for OFI Netmod"
dnl    for arg in $args_array; do
dnl    case ${arg} in
dnl      scalable-endpoints)
dnl              do_scalable_endpoints=true
dnl              echo " ---> CH4::OFI Provider : $arg"
dnl              ;;
dnl      av_table)
dnl              do_av_table=true
dnl              echo " ---> CH4::OFI Provider AV table : $arg"
dnl              ;;
dnl      direct-provider)
dnl              do_direct_provider=true
dnl              echo " ---> CH4::OFI Direct OFI Provider requested : $arg"
dnl              ;;
dnl      mr-offset)
dnl              do_mr_offset=true
dnl              echo " ---> CH4::OFI Provider Memory Registration: $arg"
dnl              ;;
dnl      static-fabric)
dnl              do_static_fabric=true
dnl              echo " ---> CH4::OFI Static library: $arg"
dnl              ;;
dnl    esac
dnl    done
dnl    IFS=$SAVE_IFS
dnl
dnl    if [test "$do_scalable_endpoints" = "true"]; then
dnl       AC_MSG_NOTICE([Enabling OFI netmod scalable endpoints])
dnl       PAC_APPEND_FLAG([-DMPIDI_USE_SCALABLE_ENDPOINTS], [CPPFLAGS])
dnl    fi
dnl
dnl    if [test "$do_av_table" = "true"]; then
dnl       AC_MSG_NOTICE([Enabling OFI netmod AV table])
dnl       PAC_APPEND_FLAG([-DMPIDI_USE_AV_TABLE], [CPPFLAGS])
dnl    fi
dnl
dnl    if [test "$do_mr_offset" = "true"]; then
dnl       AC_MSG_NOTICE([Enabling OFI netmod MR offset mode])
dnl       PAC_APPEND_FLAG([-DMPIDI_USE_MR_OFFSET], [CPPFLAGS])
dnl    fi
dnl
dnl    if [test "$do_direct_provider" = "true"]; then
dnl       AC_MSG_NOTICE([Enabling OFI netmod direct provider])
dnl       PAC_APPEND_FLAG([-DFABRIC_DIRECT],[CPPFLAGS])
dnl    fi
dnl
dnl    if [test "$do_static_fabric" = "true"]; then
dnl       MPID_LIBTOOL_STATIC_FLAG="-static -lfabric"
dnl       AC_MSG_NOTICE([Enabling static link of libfabric])
dnl    fi
])
    AM_CONDITIONAL([BUILD_CH4_NETMOD_UCX],[test "X$build_ch4_netmod_ucx" = "Xyes"])
])dnl

 AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
 AM_COND_IF([BUILD_CH4_NETMOD_UCX],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR ch4:ucx])

    PAC_SET_HEADER_LIB_PATH(ucx)
    PAC_PUSH_FLAG(LIBS)
    PAC_CHECK_HEADER_LIB_FATAL(ucx, ucp/api/ucp.h, ucp, ucp_config_read)
    PAC_POP_FLAG(LIBS)
    if [test "$enable_sharedlibs" = "osx-gcc" ]; then
        PAC_APPEND_FLAG([-Wl,-rpath -Wl,${prefix}/lib], [LDFLAGS])
        PAC_APPEND_FLAG([-Wl,-rpath -Wl,${with_ofi}/lib], [LDFLAGS])
    else
        PAC_APPEND_FLAG([-Wl,-rpath -Wl,${prefix}/lib -Wl,--enable-new-dtags], [LDFLAGS])
        PAC_APPEND_FLAG([-Wl,-rpath -Wl,${with_ucx}/lib -Wl,--enable-new-dtags], [LDFLAGS])
    fi

    if [test "$do_static_fabric" = "true"]; then
       PAC_APPEND_FLAG([-L${with_ucx}/lib],[WRAPPER_STATIC_LDFLAGS])
       PAC_APPEND_FLAG([-lucp],[WRAPPER_STATIC_LIBS])
    else
       PAC_APPEND_FLAG([-L${with_ucx}/lib -lucp],[LIBS])
    fi
    PAC_APPEND_FLAG([-lstdc++ -ldl -lpthread],[LIBS])

])dnl end AM_COND_IF(BUILD_CH4_NETMOD_OFI,...)
])dnl end _BODY

[#] end of __file__
