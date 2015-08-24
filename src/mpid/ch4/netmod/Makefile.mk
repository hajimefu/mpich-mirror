## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2012 by Argonne National Laboratory.
## (C) 2014 by Mellanox Technologies, Inc.
##     See COPYRIGHT in top-level directory.
##

AM_CPPFLAGS += -I$(top_srcdir)/src/mpid/ch4/netmod/include

noinst_HEADERS += src/mpid/ch4/netmod/include/netmod.h

include $(top_srcdir)/src/mpid/ch4/netmod/ofi/Makefile.mk
include $(top_srcdir)/src/mpid/ch4/netmod/am_ofi/Makefile.mk
include $(top_srcdir)/src/mpid/ch4/netmod/stub/Makefile.mk
