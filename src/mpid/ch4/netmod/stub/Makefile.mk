if BUILD_CH4_NETMOD_AMSTUB

mpi_core_sources += src/mpid/ch4/netmod/stub/globals.c
lib_lib@MPILIBNAME@_la_CXXFLAGS  = -fno-rtti -fno-exceptions
# errnames_txt_files += src/mpid/ch4/netmod/stub/errnames.txt

endif
