## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2011 by Argonne National Laboratory.
##     See COPYRIGHT in top-level directory.
##
if BUILD_CH4_NETMOD_OFI

AM_CPPFLAGS        += -I$(top_srcdir)/src/mpid/ch4/netmod/ofi

noinst_HEADERS     +=
mpi_core_sources   += src/mpid/ch4/netmod/ofi/func_table.c \
                      src/mpid/ch4/netmod/ofi/globals.c \
                      src/mpid/ch4/netmod/ofi/util.cc
errnames_txt_files += src/mpid/ch4/netmod/ofi/errnames.txt

if BUILD_CH4_NETMOD_OFI_STATIC
external_ldflags += -Xlinker -Bstatic -Xlinker -lfabric -Xlinker -Bdynamic -lstdc++ -ldl -lpthread
else
external_ldflags += -lfabric -lstdc++ -lpthread
endif

lib_lib@MPILIBNAME@_la_CXXFLAGS  = -fno-rtti -fno-exceptions

endif
