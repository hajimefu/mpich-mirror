[#] start of __file__
dnl MPICH_SUBCFG_BEFORE=src/mpid/common/shm
dnl MPICH_SUBCFG_AFTER=src/mpid/ch4

AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
    AM_COND_IF([BUILD_CH4],[
        for shm in $ch4_shm ; do
            AS_CASE([$shm],[simple],[build_ch4_shm_simple=yes])
        done

        AC_ARG_WITH(ch4-shmmod-simple-args,
        [  --with-ch4-shmmod-simple-args=arg1:arg2:arg3
        CH4 simple shmmod arguments:
                disable-lock-free-queues - Disable atomics and lock-free queues
                ],
                [simple_shmmod_args=$withval],
                [simple_shmmod_args=])

dnl Parse the shmmod arguments
        SAVE_IFS=$IFS
        IFS=':'
        args_array=$simple_shmmod_args
        do_disable_lock_free_queues=false
        echo "Parsing Arguments for Simple shmmod"
        for arg in $args_array; do
        case ${arg} in
            disable-lock-free-queues)
                do_disable_lock_free_queues=true
                echo " ---> CH4::SHM::SIMPLE : $arg"
                ;;
            esac
        done
        IFS=$SAVE_IFS

        if [test "$do_disable_lock_free_queues" = "true"]; then
            AC_MSG_NOTICE([Disabling simple shared memory lock free queues])
        else
            AC_MSG_NOTICE([Enabling simple shared memory lock free queues])
            PAC_APPEND_FLAG([-DMPID_NEM_USE_LOCK_FREE_QUEUES],[CPPFLAGS])
        fi
        # the simple device channel depends on the common shm code
        build_mpid_common_shm=yes
    ])
    AM_CONDITIONAL([BUILD_CH4_SHM_SIMPLE],[test "X$build_ch4_shm_simple" = "Xyes"])
])dnl

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_COND_IF([BUILD_CH4_SHM_SIMPLE],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR ch4:shm:simple])
])dnl end AM_COND_IF(BUILD_CH4_SHM_SIMPLE,...)
])dnl end _BODY

[#] end of __file__
