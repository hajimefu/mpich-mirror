## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2015 by Argonne National Laboratory.
##     See COPYRIGHT in top-level directory.
##

AM_CPPFLAGS += -I$(top_srcdir)/src/mpid/ch4/src

noinst_HEADERS += src/mpid/ch4/src/ch4_comm.h     \
                  src/mpid/ch4/src/ch4_init.h     \
                  src/mpid/ch4/src/ch4_progress.h \
                  src/mpid/ch4/src/ch4_request.h  \
                  src/mpid/ch4/src/ch4_send.h     \
                  src/mpid/ch4/src/ch4_types.h    \
                  src/mpid/ch4/src/ch4_impl.h     \
                  src/mpid/ch4/src/ch4_probe.h    \
                  src/mpid/ch4/src/ch4_proc.h     \
                  src/mpid/ch4/src/ch4_recv.h     \
                  src/mpid/ch4/src/ch4_rma.h      \
                  src/mpid/ch4/src/ch4_spawn.h    \
                  src/mpid/ch4/src/ch4_win.h      \
                  src/mpid/ch4/src/ch4u_probe.h   \
                  src/mpid/ch4/src/ch4u_recv.h    \
                  src/mpid/ch4/src/ch4r_rma.h     \
                  src/mpid/ch4/src/ch4r_win.h     \
                  src/mpid/ch4/src/ch4r_init.h    \
                  src/mpid/ch4/src/ch4u_proc.h    \
                  src/mpid/ch4/src/ch4u_recvq.h   \
                  src/mpid/ch4/src/ch4i_util.h 	  \
                  src/mpid/ch4/src/ch4i_symheap.h \
                  src/mpid/ch4/src/ch4r_send.h	  \
                  src/mpid/ch4/src/ch4r_buf.h

mpi_core_sources += src/mpid/ch4/src/ch4_globals.c        \
                    src/mpid/ch4/src/mpid_ch4_net_array.c \
                    src/mpid/ch4/src/mpid_ch4_shm_array.c \
                    src/mpid/ch4/src/ch4i_map.cc
