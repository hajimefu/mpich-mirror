/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "mpid_nem_impl.h"
#include "strings.h"

MPID_nem_netmod_funcs_t *MPID_nem_netmod_func;
int MPID_nem_netmod_id;

MPID_nem_net_module_vc_dbg_print_sendq_t  MPID_nem_net_module_vc_dbg_print_sendq = 0;

#undef FUNCNAME
#define FUNCNAME MPID_nem_choose_netmod
#undef FCNAME
#define FCNAME MPIDI_QUOTE(FUNCNAME)
int MPID_nem_choose_netmod(void)
{
    int mpi_errno = MPI_SUCCESS;
    const char *val;
    int i;
    MPIDI_STATE_DECL(MPID_STATE_MPID_NEM_CHOOSE_NETMOD);

    MPIDI_FUNC_ENTER(MPID_STATE_MPID_NEM_CHOOSE_NETMOD);

    if (! MPIU_GetEnvStr("MPICH_NEMESIS_NETMOD", &val))
    {
        /* netmod not specified, using the default */
        MPID_nem_netmod_func = MPID_nem_netmod_funcs[0];
        MPID_nem_netmod_id = i;
        goto fn_exit;
    }

    for (i = 0; i < MPID_nem_num_netmods; ++i)
    {
        if (!strncasecmp(val, MPID_nem_netmod_strings[i], MPID_NEM_MAX_NETMOD_STRING_LEN))
        {
            MPID_nem_netmod_func = MPID_nem_netmod_funcs[i];
            MPID_nem_netmod_id = i;
            goto fn_exit;
        }
    }

    MPIU_ERR_SETANDJUMP1(mpi_errno, MPI_ERR_OTHER, "**invalid_netmod", "**invalid_netmod %s", val);
 
 fn_exit:
    MPIDI_FUNC_EXIT(MPID_STATE_MPID_NEM_CHOOSE_NETMOD);
    return mpi_errno;
 fn_fail:

    goto fn_exit;
}
