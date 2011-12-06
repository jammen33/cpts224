# what flags you want to pass to the C compiler & linker
AM_CFLAGS = --pedantic -Wall -std=c99 -O2 -W
AM_LDFLAGS =

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = ma blocksize
lib_LTLIBRARIES = libcma.la
libcma_la_SOURCES = cma.c
blocksize_SOURCES = blocksize.c cma
ma_SOURCES = ma.c
ma_LDADD = libcma.la
blocksize_LDADD = libcma.la
nobase_include_HEADERS = cma/cma.h cma/debug.h