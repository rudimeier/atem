AC_PREREQ([2.61])
AC_INIT([atem], [0.3.0], [http://www.ga-group.nl/])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_SRCDIR([src/])

AM_INIT_AUTOMAKE([foreign])

## compiler check
CXXFLAGS="-O3 -g -Wall -W"
AC_PROG_CXX
AC_PROG_CC_C_O
AC_PROG_CC_C99


## tweaks
AC_ARG_ENABLE([fast-printing],[
AS_HELP_STRING([--disable-fast-printing],
    [Use slow but safe printing methods (sprintf). Default: enabled])],
	enable_fast_printing=${enableval}, enable_fast_printing="yes")
if test "${enable_fast_printing}" = "yes"; then
    AC_DEFINE([FAST_PRINTING], [1], [Define if fast printing is preferred.])
fi


dnl AC_DEFINE([FAST_PRINTING], [1], [Define if fast printing is preferred.]

## output
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT

