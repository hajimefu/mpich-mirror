## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2014 by Argonne National Laboratory.
##     See COPYRIGHT in top-level directory.
##

AM_CPPFLAGS += -I$(top_srcdir)/src/mpid/ch4/shm/include

noinst_HEADERS += src/mpid/ch4/shm/include/ch4_shm_api.h    \
                  src/mpid/ch4/shm/include/ch4_shm.h        \
                  src/mpid/ch4/shm/include/ch4_shm_macros.h
