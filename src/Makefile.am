AM_CPPFLAGS = -DDEBUG -D_GNU_SOURCE
AM_LDFLAGS =

headerdir = $(includedir)/atem

bin_PROGRAMS =
noinst_PROGRAMS =
lib_LTLIBRARIES =
header_HEADERS =
noinst_LIBRARIES =

bin_PROGRAMS += atem
atem_SOURCES = metastock.h ms_file.h util.h
atem_SOURCES += atem.cpp
atem_SOURCES += metastock.cpp
atem_SOURCES += ms_file.cpp
atem_SOURCES += util.cpp
atem_LDADD = -lpopt
atem_CPPFLAGS = $(AM_CPPFLAGS)
atem_LDFLAGS = $(AM_LDFLAGS)

# 
# Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(cflags) $<

