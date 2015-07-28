## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2014 by Argonne National Laboratory.
##     See COPYRIGHT in top-level directory.
##

if BUILD_CH4_SHM

mpi_core_sources += src/mpid/ch4/shm/ch4_shm_globals.c \
		    src/mpid/ch4/shm/ch4_shm_func_table.c \
		    src/mpid/ch4/shm/ch4_shm_barrier.c \
		    src/mpid/ch4/shm/ch4_shm_alloc.c

AM_CPPFLAGS += -I$(top_srcdir)/src/mpid/ch4/shm/include   \
               -I$(top_builddir)/src/mpid/ch4/shm/include

noinst_HEADERS += src/mpid/ch4/shm/include/ch4_shm.h


lib_lib@MPILIBNAME@_la_CXXFLAGS  = -fno-rtti -fno-exceptions
# errnames_txt_files += src/mpid/ch4/shm/errnames.txt

endif
