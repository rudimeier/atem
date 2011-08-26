AM_CPPFLAGS = -DDEBUG -D_GNU_SOURCE
AM_LDFLAGS =

headerdir = $(includedir)/atem

EXTRA_DIST =
EXTRA_DIST += atem.ggo
EXTRA_DIST += $(BUILT_SOURCES)

bin_PROGRAMS =
bin_PROGRAMS += atem
atem_SOURCES =
atem_SOURCES += atem.cpp
atem_SOURCES += metastock.cpp
atem_SOURCES += ms_file.cpp
atem_SOURCES += util.cpp
noinst_HEADERS =
noinst_HEADERS += metastock.h ms_file.h util.h
EXTRA_atem_SOURCES =
EXTRA_atem_SOURCES += ftoa.c
EXTRA_atem_SOURCES += itoa.c
BUILT_SOURCES =
BUILT_SOURCES += atem_ggo.c atem_ggo.h
atem_CPPFLAGS = $(AM_CPPFLAGS)
atem_LDFLAGS = $(AM_LDFLAGS)

CLEANFILES =
CLEANFILES += *.s
CLEANFILES += *.i


%_ggo.c %_ggo.h: %.ggo
	gengetopt --no-help --no-version -i $< -F $*_ggo

# 
# Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<
.cpp.i:
	$(CXXCOMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(cflags) $<
.cpp.s:
	$(CXXCOMPILE) -S -c $(cflags) $<

