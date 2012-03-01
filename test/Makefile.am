# Help the Developers and yourself. Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG = C
LC_ALL = C

EXTRA_DIST = $(TESTS)
EXTRA_DIST += $(ATST_LOG_COMPILER)
TESTS =

TEST_EXTENSIONS = .atst
ATST_LOG_COMPILER = $(srcdir)/atem-test.sh
AM_ATST_LOG_FLAGS = --builddir $(top_builddir)/src --hash sha1sum
LOG_COMPILER = echo

EXTRA_DIST += msdir_equis_a.tar.xz

all-local:
	tar -xf $(srcdir)/msdir_equis_a.tar.xz

clean-local:
	-rm -rf msdir_equis_a
