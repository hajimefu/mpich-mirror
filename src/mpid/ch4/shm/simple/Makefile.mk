## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2014 by Argonne National Laboratory.
##     See COPYRIGHT in top-level directory.
##
##  Portions of this code were written by Intel Corporation.
##  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
##  to Argonne National Laboratory subject to Software Grant and Corporate
##  Contributor License Agreement dated February 8, 2012.
##

if BUILD_CH4_SHM_SIMPLE

noinst_HEADERS += src/mpid/ch4/shm/simple/am.h        \
                  src/mpid/ch4/shm/simple/coll.h      \
                  src/mpid/ch4/shm/simple/datatypes.h \
                  src/mpid/ch4/shm/simple/shm_direct.h    \
                  src/mpid/ch4/shm/simple/init.h      \
                  src/mpid/ch4/shm/simple/progress.h  \
                  src/mpid/ch4/shm/simple/recv.h      \
                  src/mpid/ch4/shm/simple/rma.h       \
                  src/mpid/ch4/shm/simple/spawn.h     \
                  src/mpid/ch4/shm/simple/win.h       \
                  src/mpid/ch4/shm/simple/comm.h      \
                  src/mpid/ch4/shm/simple/defs.h      \
                  src/mpid/ch4/shm/simple/impl.h      \
                  src/mpid/ch4/shm/simple/probe.h     \
                  src/mpid/ch4/shm/simple/queue.h     \
                  src/mpid/ch4/shm/simple/request.h   \
                  src/mpid/ch4/shm/simple/send.h      \
                  src/mpid/ch4/shm/simple/unimpl.h

mpi_core_sources += src/mpid/ch4/shm/simple/globals.c    \
                    src/mpid/ch4/shm/simple/func_table.c \
                    src/mpid/ch4/shm/simple/barrier.c

endif
