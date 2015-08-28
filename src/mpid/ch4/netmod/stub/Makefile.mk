if BUILD_CH4_NETMOD_STUB

AM_CPPFLAGS        += -I$(top_srcdir)/src/mpid/ch4/netmod/stub

mpi_core_sources += src/mpid/ch4/netmod/stub/globals.c
# errnames_txt_files += src/mpid/ch4/netmod/stub/errnames.txt

endif
