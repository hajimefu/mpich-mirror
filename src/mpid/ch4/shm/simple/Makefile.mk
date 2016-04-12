## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2014 by Argonne National Laboratory.
##     See COPYRIGHT in top-level directory.
##

if BUILD_CH4_SHM_SIMPLE

AM_CPPFLAGS += -I$(top_srcdir)/src/mpid/ch4/shm/simple

noinst_HEADERS += src/mpid/ch4/shm/simple/ch4_shm_am.h        \
                  src/mpid/ch4/shm/simple/ch4_shm_coll.h      \
                  src/mpid/ch4/shm/simple/ch4_shm_datatypes.h \
                  src/mpid/ch4/shm/simple/ch4_shm_direct.h    \
                  src/mpid/ch4/shm/simple/ch4_shm_init.h      \
                  src/mpid/ch4/shm/simple/ch4_shm_progress.h  \
                  src/mpid/ch4/shm/simple/ch4_shm_recv.h      \
                  src/mpid/ch4/shm/simple/ch4_shm_rma.h       \
                  src/mpid/ch4/shm/simple/ch4_shm_spawn.h     \
                  src/mpid/ch4/shm/simple/ch4_shm_win.h       \
                  src/mpid/ch4/shm/simple/ch4_shm_comm.h      \
                  src/mpid/ch4/shm/simple/ch4_shm_defs.h      \
                  src/mpid/ch4/shm/simple/ch4_shm_impl.h      \
                  src/mpid/ch4/shm/simple/ch4_shm_probe.h     \
                  src/mpid/ch4/shm/simple/ch4_shm_queue.h     \
                  src/mpid/ch4/shm/simple/ch4_shm_request.h   \
                  src/mpid/ch4/shm/simple/ch4_shm_send.h      \
                  src/mpid/ch4/shm/simple/ch4_shm_unimpl.h

mpi_core_sources += src/mpid/ch4/shm/simple/ch4_shm_globals.c    \
                    src/mpid/ch4/shm/simple/ch4_shm_func_table.c \
                    src/mpid/ch4/shm/simple/ch4_shm_barrier.c

endif
