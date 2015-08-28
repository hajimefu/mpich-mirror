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

    if test -z "$ch4_netmod_pre_include" ; then
        ch4_netmod_pre_include="#include \"${net}_pre.h\""
    else
        ch4_netmod_pre_include="${ch4_netmod_pre_include}
        #include \"${net}_pre.h\""
    fi

    if test -z "$ch4_netmod_amrequest_decl" ; then
        ch4_netmod_amrequest_decl="MPIDI_netmod_${net}_amrequest_t ${net};"
    else
        ch4_netmod_amrequest_decl="${ch4_netmod_amrequest_decl} \\
MPIDI_netmod_${net}_amrequest_t ${net};"
    fi

    if test -z "$ch4_netmod_request_decl" ; then
        ch4_netmod_request_decl="MPIDI_netmod_${net}_request_t ${net};"
    else
        ch4_netmod_request_decl="${ch4_netmod_request_decl} \\
MPIDI_netmod_${net}_request_t ${net};"
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
AC_SUBST(ch4_netmod_pre_include)
AC_SUBST(ch4_netmod_amrequest_decl)
AC_SUBST(ch4_netmod_request_decl)
AM_SUBST_NOTMAKE(ch4_netmod_pre_include)
AM_SUBST_NOTMAKE(ch4_netmod_amrequest_decl)
AM_SUBST_NOTMAKE(ch4_netmod_request_decl)

if test "$ch4_nets_array_sz" == "1" ;  then
   PAC_APPEND_FLAG([-DNETMOD_DIRECT=__netmod_direct_${ch4_netmods}__], [CPPFLAGS])
fi


AC_ARG_ENABLE(ch4-shm,
    [--enable-ch4-shm=level:module
       Control whether CH4 shared memory is built and/or used.
  /home/cjarcher/code/mpich/ssg_sfi-mpich/src/mpid/ch4/     level:
         no        - Do not build or use CH4 shared memory.
         yes       - Build CH4 shared memory, but do not use it by default (Your chosen netmod must provide it).
         exclusive - Build and exclusively use CH4 shared memory. (Default)
  /home/cjarcher/code/mpich/ssg_sfi-mpich/src/mpid/ch4/     module-list(optional).  comma separated list of shared memory modules:
         simple   - simple shared memory implementation
    ],,enable_ch4_shm=exclusive:simple)

ch4_shm_level=`echo $enable_ch4_shm | sed -e 's/:.*$//'`
changequote(<<,>>)
ch4_shm=`echo $enable_ch4_shm | sed -e 's/^[^:]*//' -e 's/^://'`
changequote([,])

if test "$ch4_shm_level" != "no" ; then
    AC_DEFINE([MPIDI_BUILD_CH4_SHM], [1],
        [Define if CH4 will build the default shared memory implementation as opposed to only using a netmod implementation])
else
    ch4_shm=
fi

if test "$ch4_shm_level" = "exclusive" ; then
    AC_DEFINE([MPIDI_CH4_EXCLUSIVE_SHM], [1],
        [Define if CH4 will be providing the exclusive implementation of shared memory])
fi

# $ch4_shm - contains the shm mods
if test -z "${ch4_shm}" ; then
   if test "$ch4_shm_level" != "no" ; then
      ch4_shm="simple"
   fi
else
   ch4_shm=`echo ${ch4_shm} | sed -e 's/,/ /g'`
fi
export ch4_shm

ch4_shm_func_decl=""
ch4_shm_native_func_decl=""
ch4_shm_func_array=""
ch4_shm_native_func_array=""
ch4_shm_strings=""
shm_index=0
for shm in $ch4_shm ; do
    if test ! -d $srcdir/src/mpid/ch4/shm/${shm} ; then
        AC_MSG_ERROR([Shared memory module ${shm} is unknown "$srcdir/src/mpid/ch4/shm/${shm}"])
    fi
    shm_macro=`echo $shm | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
    shm_macro="MPIDI_CH4_SHM_${shm_macro}"

    if test -z "$ch4_shm_array" ; then
        ch4_shm_array="$shm_macro"
    else
        ch4_shm_array="$ch4_shm_array, $shm_macro"
    fi

    if test -z "$ch4_shm_func_decl" ; then
        ch4_shm_func_decl="MPIDI_shm_${shm}_funcs"
    else
        ch4_shm_func_decl="${ch4_shm_func_decl}, MPIDI_shm_${shm}_funcs"
    fi

    if test -z "$ch4_shm_native_func_decl" ; then
        ch4_shm_native_func_decl="MPIDI_shm_native_${shm}_funcs"
    else
        ch4_shm_native_func_decl="${ch4_shm_native_func_decl}, MPIDI_shm_native_${shm}_funcs"
    fi

    if test -z "$ch4_shm_func_array" ; then
        ch4_shm_func_array="&MPIDI_shm_${shm}_funcs"
    else
        ch4_shm_func_array="${ch4_shm_func_array}, &MPIDI_shm_${shm}_funcs"
    fi

    if test -z "$ch4_shm_native_func_array" ; then
        ch4_shm_native_func_array="&MPIDI_shm_native_${shm}_funcs"
    else
        ch4_shm_native_func_array="${ch4_shm_native_func_array}, &MPIDI_shm_native_${shm}_funcs"
    fi

    if test -z "$ch4_shm_strings" ; then
        ch4_shm_strings="\"$shm\""
    else
        ch4_shm_strings="$ch4_shm_strings, \"$shm\""
    fi

    if test -z "$ch4_shm_pre_include" ; then
        ch4_shm_pre_include="#include \"shm_${shm}_pre.h\""
    else
        ch4_shm_pre_include="${ch4_shm_pre_include}
        #include \"shm_${shm}_pre.h\""
    fi

    if test -z "$ch4_shm_request_decl" ; then
        ch4_shm_request_decl="MPIDI_shm_${shm}_request_t ${shm};"
    else
        ch4_shm_request_decl="${ch4_shm_request_decl} \\
MPIDI_shm_${shm}_request_t ${shm};"
    fi


    shm_index=`expr $shm_index + 1`
done
ch4_shm_array_sz=$shm_index

AC_SUBST(ch4_shm)
AC_SUBST(ch4_shm_array)
AC_SUBST(ch4_shm_array_sz)
AC_SUBST(ch4_shm_func_decl)
AC_SUBST(ch4_shm_native_func_decl)
AC_SUBST(ch4_shm_func_array)
AC_SUBST(ch4_shm_native_func_array)
AC_SUBST(ch4_shm_strings)
AC_SUBST(ch4_shm_pre_include)
AC_SUBST(ch4_shm_request_decl)
AM_SUBST_NOTMAKE(ch4_shm_pre_include)
AM_SUBST_NOTMAKE(ch4_shm_request_decl)

if test "$ch4_shm_array_sz" == "1" ;  then
   PAC_APPEND_FLAG([-DSHM_DIRECT=__shm_direct_${ch4_shm}__], [CPPFLAGS])
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

AC_ARG_ENABLE(ch4u-per-comm-msg-queue,
    [--enable-ch4u-per-comm-msg-queue=option
       Enable use of per-communicator message queues for posted recvs/unexpected messages
         yes       - Use per-communicator message queue. (Default)
         no        - Use global queue for posted recvs/unexpected messages.
    ],,enable_ch4u_per_comm_msg_queue=yes)

if test "$enable_ch4u_per_comm_msg_queue" = "yes" ; then
    AC_DEFINE([MPIDI_CH4U_USE_PER_COMM_QUEUE], [1],
        [Define if CH4U will use per-communicator message queues])
fi

AC_CONFIG_FILES([
src/mpid/ch4/src/mpid_ch4_net_array.c
src/mpid/ch4/src/mpid_ch4_shm_array.c
src/mpid/ch4/include/netmodpre.h
])

])dnl end AM_COND_IF(BUILD_CH4,...)

AM_CONDITIONAL([BUILD_CH4_SHM],[test "$ch4_shm_level" = "yes" -o "$ch4_shm_level" = "exclusive"])


])dnl end _BODY

[#] end of __file__
