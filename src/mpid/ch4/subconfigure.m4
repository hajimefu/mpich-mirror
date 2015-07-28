[#] start of __file__
dnl MPICH_SUBCFG_BEFORE=src/mpid/common/sched
dnl MPICH_SUBCFG_BEFORE=src/mpid/common/datatype
dnl MPICH_SUBCFG_BEFORE=src/mpid/common/thread

dnl _PREREQ handles the former role of mpichprereq, setup_device, etc
[#] expansion is: PAC_SUBCFG_PREREQ_[]PAC_SUBCFG_AUTO_SUFFIX
AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_CONDITIONAL([BUILD_CH4],[test "$device_name" = "ch4"])

# the CH4 device depends on the common NBC scheduler code
build_mpid_common_sched=yes
build_mpid_common_datatype=yes
build_mpid_common_thread=yes

MPID_MAX_THREAD_LEVEL=MPI_THREAD_MULTIPLE

AM_COND_IF([BUILD_CH4],[
AC_MSG_NOTICE([RUNNING PREREQ FOR CH4 DEVICE])

# $device_args - contains the netmods
if test -z "${device_args}" ; then
    ch4_netmods="ofi"
else
    ch4_netmods=`echo ${device_args} | sed -e 's/,/ /g'`
fi
export ch4_netmods

#
# reset DEVICE so that it (a) always includes the channel name, and (b) does not include channel options
#
DEVICE="${device_name}:${ch4_netmods}"

ch4_nets_func_decl=""
ch4_nets_native_func_decl=""
ch4_nets_func_array=""
ch4_nets_native_func_array=""
ch4_nets_strings=""
net_index=0
for net in $ch4_netmods ; do
    if test ! -d $srcdir/src/mpid/ch4/netmod/${net} ; then
        AC_MSG_ERROR([Network module ${net} is unknown "$srcdir/src/mpid/ch4/netmod/${net}"])
    fi
    net_macro=`echo $net | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
    net_macro="MPIDI_CH4_${net_macro}"

    if test -z "$ch4_nets_array" ; then
        ch4_nets_array="$net_macro"
    else
        ch4_nets_array="$ch4_nets_array, $net_macro"
    fi

    if test -z "$ch4_nets_func_decl" ; then
        ch4_nets_func_decl="MPIDI_netmod_${net}_funcs"
    else
        ch4_nets_func_decl="${ch4_nets_func_decl}, MPIDI_netmod_${net}_funcs"
    fi

    if test -z "$ch4_nets_native_func_decl" ; then
        ch4_nets_native_func_decl="MPIDI_netmod_native_${net}_funcs"
    else
        ch4_nets_native_func_decl="${ch4_nets_native_func_decl}, MPIDI_netmod_native_${net}_funcs"
    fi

    if test -z "$ch4_nets_func_array" ; then
        ch4_nets_func_array="&MPIDI_netmod_${net}_funcs"
    else
        ch4_nets_func_array="${ch4_nets_func_array}, &MPIDI_netmod_${net}_funcs"
    fi

    if test -z "$ch4_nets_native_func_array" ; then
        ch4_nets_native_func_array="&MPIDI_netmod_native_${net}_funcs"
    else
        ch4_nets_native_func_array="${ch4_nets_native_func_array}, &MPIDI_netmod_native_${net}_funcs"
    fi

    if test -z "$ch4_nets_strings" ; then
        ch4_nets_strings="\"$net\""
    else
        ch4_nets_strings="$ch4_nets_strings, \"$net\""
    fi

    net_index=`expr $net_index + 1`
done
ch4_nets_array_sz=$net_index

AC_SUBST(device_name)
AC_SUBST(ch4_netmods)
AC_SUBST(ch4_nets_array)
AC_SUBST(ch4_nets_array_sz)
AC_SUBST(ch4_nets_func_decl)
AC_SUBST(ch4_nets_native_func_decl)
AC_SUBST(ch4_nets_func_array)
AC_SUBST(ch4_nets_native_func_array)
AC_SUBST(ch4_nets_strings)

if test "$ch4_nets_array_sz" == "1" ;  then
   PAC_APPEND_FLAG([-DNETMOD_DIRECT=__netmod_direct_${ch4_netmods}__], [CPPFLAGS])
fi


])dnl end AM_COND_IF(BUILD_CH4,...)
])dnl end PREREQ

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_COND_IF([BUILD_CH4],[
AC_MSG_NOTICE([RUNNING CONFIGURE FOR CH4 DEVICE])

AC_ARG_WITH(ch4-rank-bits, [--with-ch4-rank-bits=16/32     Number of bits allocated to the rank field (16 or 32)],
			   [ rankbits=$withval ],
			   [ rankbits=32 ])
if test "$rankbits" != "16" -a "$rankbits" != "32" ; then
   AC_MSG_ERROR(Only 16 or 32-bit ranks are supported)
fi
AC_DEFINE_UNQUOTED(CH4_RANK_BITS,$rankbits,[Define the number of CH4_RANK_BITS])

AC_ARG_ENABLE(ch4-shm,
    [--enable-ch4-shm=level
       Control whether CH4 shared memory is built and/or used.
         no        - Do not build or use CH4 shared memory.
         yes       - Build CH4 shared memory, but do not use it by default (Your chosen netmod must provide it).
         exclusive - Build and exclusively use CH4 shared memory. (Default)
    ],,enable_ch4_shm=exclusive)

if test "$enable_ch4_shm" != "no" ; then
    AC_DEFINE([MPIDI_BUILD_CH4_SHM], [1],
        [Define if CH4 will build the default shared memory implementation as opposed to only using a netmod implementation])
fi

if test "$enable_ch4_shm" = "exclusive" ; then
    AC_DEFINE([MPIDI_CH4_EXCLUSIVE_SHM], [1],
        [Define if CH4 will be providing the exclusive implementation of shared memory])
fi

AC_CONFIG_FILES([
src/mpid/ch4/src/mpid_ch4_net_array.c
])

])dnl end AM_COND_IF(BUILD_CH4,...)

AM_CONDITIONAL([BUILD_CH4_SHM],[test "$enable_ch4_shm" = "yes" -o "$enable_ch4_shm" = "exclusive"])

])dnl end _BODY

[#] end of __file__
