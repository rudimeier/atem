# Help the Developers and yourself. Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG = C
LC_ALL = C

BUILT_SOURCES =
BUILT_SOURCES += $(ms_dirs)
EXTRA_DIST = $(TESTS)
EXTRA_DIST += $(ATST_LOG_COMPILER)
EXTRA_DIST += $(patsubst %,%.tar.xz,$(ms_dirs))
TESTS =

TEST_EXTENSIONS = .atst
ATST_LOG_COMPILER = $(srcdir)/atem-test.sh
AM_ATST_LOG_FLAGS = --builddir $(top_builddir)/src --hash sha1sum
LOG_COMPILER = echo

ms_dirs =
ms_dirs += msdir_equis_a

TESTS += equis.01.atst
TESTS += equis.02.atst
TESTS += equis.03.atst
TESTS += equis.04.atst
TESTS += equis.05.atst
TESTS += equis.06.atst
TESTS += equis.07.atst
TESTS += equis.08.atst
TESTS += float-x.01.atst
TESTS += float-x.02.atst
TESTS += format.01.atst
TESTS += format.02.atst
TESTS += format.03.atst
TESTS += format.04.atst
TESTS += format.05.atst
TESTS += format.06.atst

msdir_%: msdir_%.tar.xz
	xz -dc $< | $(am__untar) && touch $@

clean-local:
	-rm -rf msdir_equis_a

# don't run the tests parallel
.NOTPARALLEL:
