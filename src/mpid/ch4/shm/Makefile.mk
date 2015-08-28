## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2012 by Argonne National Laboratory.
## (C) 2014 by Mellanox Technologies, Inc.
##     See COPYRIGHT in top-level directory.
##


if BUILD_CH4_SHM

include $(top_srcdir)/src/mpid/ch4/shm/include/Makefile.mk
include $(top_srcdir)/src/mpid/ch4/shm/stub/Makefile.mk
include $(top_srcdir)/src/mpid/ch4/shm/simple/Makefile.mk

endif
