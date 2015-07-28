if BUILD_CH4_NETMOD_AM_OFI

mpi_core_sources += src/mpid/ch4/netmod/am_ofi/func_table.c \
			src/mpid/ch4/netmod/am_ofi/globals.c \
			src/mpid/ch4/netmod/am_ofi/util.cc
lib_lib@MPILIBNAME@_la_CXXFLAGS  = -fno-rtti -fno-exceptions
# errnames_txt_files += src/mpid/ch4/netmod/am_ofi/errnames.txt

endif
