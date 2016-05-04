if BUILD_CH4_SHM_STUB

mpi_core_sources += src/mpid/ch4/shm/stub/globals.c    \
                    src/mpid/ch4/shm/stub/func_table.c
lib_lib@MPILIBNAME@_la_CXXFLAGS  = -fno-rtti -fno-exceptions
# errnames_txt_files += src/mpid/ch4/shm/stub/errnames.txt

endif
