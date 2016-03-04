if BUILD_CH4_NETMOD_PORTALS4

AM_CPPFLAGS        += -I$(top_srcdir)/src/mpid/ch4/netmod/portals4

mpi_core_sources   += src/mpid/ch4/netmod/portals4/globals.c \
                      src/mpid/ch4/netmod/portals4/func_table.c
errnames_txt_files += src/mpid/ch4/netmod/portals4/errnames.txt

endif
