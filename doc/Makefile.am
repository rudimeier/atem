# Help the Developers and yourself.  Just use the C locale and settings
# for the compilation. They can still be overridden by make LANG=<whatever>
# but that is general a not very good idea
LANG=C
LC_ALL=C

BUILT_SOURCES =
EXTRA_DIST = $(BUILT_SOURCES)

man1_MANS =
man1_MANS += $(built_mans)

EXTRA_DIST += author.h2m
BUILT_SOURCES += $(built_mans)

built_mans =
built_mans += atem.1

## non generic deps per executable
atem.1: $(top_srcdir)/src/atem.cpp

## help2man helpers
%.1: $(top_srcdir)/src/%.ggo $(top_srcdir)/configure
	help_opt="--help"; \
	$(top_builddir)/src/$* --full-help >/dev/null 2>&1 && help_opt="--full-help"; \
	$(HELP2MAN) -S "$(PACKAGE_STRING)" -N \
		--help-option=$$help_opt \
		-I "$(srcdir)/$*.h2m" -I "$(builddir)/$*.h2m" \
		-I "$(srcdir)/author.h2m" \
		-o $@ $(top_builddir)/src/$*
