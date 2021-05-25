#!/usr/bin/make
#
# Makefile para GNU/Linux u otro sistema GNU/*
#
ifdef DEBUG
CFLAGS1=-O2 -Wall -Werror -g3 -I.
else
CFLAGS1=-O2 -I.
endif

ifdef COMPRESS
CFLAGS=$(CFLAGS1) -DENABLE_COMPRESS=1
else
CFLAGS=$(CFLAGS1)
endif

CXXFLAGS=$(CFLAGS)
LDLIBS=

all: libcppfio.a cppfio_config

communic.o: communic.h cppfio.h excp.h
cserie.o: cserie.h communic.h cppfio.h excp.h
ptimer.o: ptimer.h cppfio.h excp.h
excp.o: cppfio.h excp.h
chrono.o: chrono.h cppfio.h
pregex.o: pregex.h cppfio.h

libcppfio.a: excp.o file.o cmdline.o errlog.o helpers.o \
	communic.o cserie.o ptimer.o chrono.o text_file.o strtok.o pregex.o
	ar rcs libcppfio.a $^

clean:
	-@rm -f *.o *.a .*~ cppfio_config

