## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2014 by Argonne National Laboratory.
##     See COPYRIGHT in top-level directory.
##

if BUILD_CH4

AM_CPPFLAGS += -I$(top_srcdir)/src/mpid/ch4/include   \
               -I$(top_builddir)/src/mpid/ch4/include

noinst_HEADERS += src/mpid/ch4/include/mpidimpl.h \
		  src/mpid/ch4/include/mpidpost.h \
		  src/mpid/ch4/include/mpidpre.h


include $(top_srcdir)/src/mpid/ch4/src/Makefile.mk
include $(top_srcdir)/src/mpid/ch4/netmod/Makefile.mk
include $(top_srcdir)/src/mpid/ch4/shm/Makefile.mk

endif BUILD_CH4
