[#] start of __file__
dnl MPICH_SUBCFG_AFTER=src/mpid/ch4

AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
    AM_COND_IF([BUILD_CH4],[
        for net in $ch4_netmods ; do
            AS_CASE([$net],[ofi],[build_ch4_netmod_ofi=yes])
	    if test $net = "ofi" ; then
	       AC_DEFINE(HAVE_CH4_NETMOD_OFI,1,[OFI netmod is built])
	       if test "$build_ch4_locality_info" != "yes" ; then
	       	  AC_DEFINE(MPIDI_BUILD_CH4_LOCALITY_INFO, 1, [CH4 should build locality info])
		  build_ch4_locality_info="yes"
	       fi
	    fi
        done

        AC_ARG_WITH(ch4-netmod-ofi-args,
        [  --with-ch4-netmod-ofi-args=arg1:arg2:arg3
        CH4 OFI netmod arguments:
                scalable-endpoints - Use OFI scalable endpoint mode
                av-table           - Use OFI AV table (logical addressing mode).  Default is av-map mode
                mr-offset          - USE OFI memory region offset mode
                direct-provider    - USE OFI FI_DIRECT to compile in a single OFI direct provider
                static-fabric      - Statically link libfabric
                ],
                [ofi_netmod_args=$withval],
                [ofi_netmod_args=])

dnl Parse the device arguments
    SAVE_IFS=$IFS
    IFS=':'
    args_array=$ofi_netmod_args
    do_scalable_endpoints=false
    do_direct_provider=false
    do_mr_offset=false
    do_av_table=false
    do_static_fabric=false
    echo "Parsing Arguments for OFI Netmod"
    for arg in $args_array; do
    case ${arg} in
      scalable-endpoints)
              do_scalable_endpoints=true
              echo " ---> CH4::OFI Provider : $arg"
              ;;
      av_table)
              do_av_table=true
              echo " ---> CH4::OFI Provider AV table : $arg"
              ;;
      direct-provider)
              do_direct_provider=true
              echo " ---> CH4::OFI Direct OFI Provider requested : $arg"
              ;;
      mr-offset)
              do_mr_offset=true
              echo " ---> CH4::OFI Provider Memory Registration: $arg"
              ;;
      static-fabric)
              do_static_fabric=true
              echo " ---> CH4::OFI Static library: $arg"
              ;;
    esac
    done
    IFS=$SAVE_IFS

    if [test "$do_scalable_endpoints" = "true"]; then
       AC_MSG_NOTICE([Enabling OFI netmod scalable endpoints])
       PAC_APPEND_FLAG([-DMPIDI_USE_SCALABLE_ENDPOINTS], [CPPFLAGS])
    fi

    if [test "$do_av_table" = "true"]; then
       AC_MSG_NOTICE([Enabling OFI netmod AV table])
       PAC_APPEND_FLAG([-DMPIDI_USE_AV_TABLE], [CPPFLAGS])
    fi

    if [test "$do_mr_offset" = "true"]; then
       AC_MSG_NOTICE([Enabling OFI netmod MR offset mode])
       PAC_APPEND_FLAG([-DMPIDI_USE_MR_OFFSET], [CPPFLAGS])
    fi

    if [test "$do_direct_provider" = "true"]; then
       AC_MSG_NOTICE([Enabling OFI netmod direct provider])
       PAC_APPEND_FLAG([-DFABRIC_DIRECT],[CPPFLAGS])
    fi

    if [test "$do_static_fabric" = "true"]; then
       MPID_LIBTOOL_STATIC_FLAG="-static -lfabric"
       AC_MSG_NOTICE([Enabling static link of libfabric])
    fi
])
    AM_CONDITIONAL([BUILD_CH4_NETMOD_OFI],[test "X$build_ch4_netmod_ofi" = "Xyes"])
])dnl

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_COND_IF([BUILD_CH4_NETMOD_OFI],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR ch4:ofi])

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
       PAC_APPEND_FLAG([-lstdc++ -lpthread -ldl -lrt],[WRAPPER_LIBS])
       PAC_APPEND_FLAG([-L${with_ofi}/lib],[WRAPPER_STATIC_LDFLAGS])
       PAC_APPEND_FLAG([-lfabric],[WRAPPER_STATIC_LIBS])
       PAC_APPEND_FLAG([-lstdc++ -lpthread -ldl -lrt],[LIBS])
    else
       PAC_APPEND_FLAG([-lfabric -lstdc++ -lpthread -ldl -lrt],[WRAPPER_LIBS])
       PAC_APPEND_FLAG([-lfabric -lstdc++ -lpthread -ldl -lrt],[LIBS])
    fi
])dnl end AM_COND_IF(BUILD_CH4_NETMOD_OFI,...)
])dnl end _BODY

[#] end of __file__
