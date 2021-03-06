## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2012 by Argonne National Laboratory.
## (C) 2014 by Mellanox Technologies, Inc.
##     See COPYRIGHT in top-level directory.
##
##  Portions of this code were written by Intel Corporation.
##  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
##  to Argonne National Laboratory subject to Software Grant and Corporate
##  Contributor License Agreement dated February 8, 2012.
##

AM_CPPFLAGS += -I$(top_srcdir)/src/mpid/ch4/netmod/include

noinst_HEADERS += src/mpid/ch4/netmod/include/netmod.h
noinst_HEADERS += src/mpid/ch4/netmod/include/netmod_impl.h

include $(top_srcdir)/src/mpid/ch4/netmod/ofi/Makefile.mk
include $(top_srcdir)/src/mpid/ch4/netmod/ucx/Makefile.mk
include $(top_srcdir)/src/mpid/ch4/netmod/portals4/Makefile.mk
include $(top_srcdir)/src/mpid/ch4/netmod/stubnm/Makefile.mk
