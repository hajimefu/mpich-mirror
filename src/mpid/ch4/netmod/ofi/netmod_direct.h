/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2012 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef NETMOD_DIRECT_H_INCLUDED
#define NETMOD_DIRECT_H_INCLUDED

#ifdef USE_OFI_TAGGED
#include "init.h"
#include "probe.h"
#include "progress.h"
#include "recv.h"
#include "request.h"
#include "send.h"
#include "win.h"
#include "rma.h"
#include "am.h"
#include "spawn.h"
#include "comm.h"
#include "unimpl.h"
#include "proc.h"
#else
#include "am_init.h"
#include "am_probe.h"
#include "am_progress.h"
#include "am_recv.h"
#include "am_request.h"
#include "am_send.h"
#include "am_win.h"
#include "am_rma.h"
#include "am_am.h"
#include "am_spawn.h"
#include "am_comm.h"
#include "unimpl.h"
#include "am_proc.h"
#endif /* USE_OFI_TAGGED */

#endif /* NETMOD_DIRECT_H_INCLUDED */
