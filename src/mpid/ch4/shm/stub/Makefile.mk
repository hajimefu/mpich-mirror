if BUILD_CH4_SHM_STUB

AM_CPPFLAGS        += -I$(top_srcdir)/src/mpid/ch4/shm/stub

mpi_core_sources += src/mpid/ch4/shm/stub/ch4_shm_globals.c    \
                    src/mpid/ch4/shm/stub/ch4_shm_func_table.c
lib_lib@MPILIBNAME@_la_CXXFLAGS  = -fno-rtti -fno-exceptions
# errnames_txt_files += src/mpid/ch4/shm/stub/errnames.txt

endif
