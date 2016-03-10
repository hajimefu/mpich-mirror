## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2011 by Argonne National Laboratory.
##     See COPYRIGHT in top-level directory.
##
if BUILD_CH4_NETMOD_UCX

AM_CPPFLAGS        += -I$(top_srcdir)/src/mpid/ch4/netmod/ucx/

noinst_HEADERS     +=
mpi_core_sources   += src/mpid/ch4/netmod/ucx/func_table.c\
                      src/mpid/ch4/netmod/ucx/global.c\
                      src/mpid/ch4/netmod/ucx/util.c

errnames_txt_files += src/mpid/ch4/netmod/ucx/errnames.txt

endif
