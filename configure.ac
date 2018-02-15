AC_PREREQ([2.61])
AC_INIT([atem], [0.4.0], [http://www.ga-group.nl/])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_SRCDIR([src/])

AM_INIT_AUTOMAKE([1.11 -Wall -Wno-portability foreign dist-xz no-dist-gzip
                  parallel-tests color-tests])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

if test -z "$CXXFLAGS"; then
	CXXFLAGS="-O3 -g"
fi

## compiler check
AC_PROG_CC
AC_PROG_CC_C_O
AC_PROG_CC_C99
AC_PROG_CXX
AC_PROG_CXX_C_O

AC_LANG([C++])
AX_COMPILER_VENDOR
# enable some warnings
AX_CHECK_COMPILE_FLAG([-Wall],[warnflags="-Wall"])
AX_CHECK_COMPILE_FLAG([-Wextra],[warnflags="$warnflags -Wextra"])
# intel's -Wextra is too verbose, disable some remarks
if test "$ax_cv_cxx_compiler_vendor" = "intel"; then
	AX_CHECK_COMPILE_FLAG([-diag-disable=11074],[warnflags="$warnflags -diag-disable=11074,11076"])
fi
CXXFLAGS="$warnflags $CXXFLAGS"

AC_C_BIGENDIAN()

## check for byteorder utils
AC_CHECK_HEADERS([endian.h sys/endian.h byteorder.h byteswap.h])

## tweaks
AC_ARG_ENABLE([fast-printing],[
AS_HELP_STRING([--disable-fast-printing],
    [Use slow but safe printing methods (sprintf). Default: enabled])],
	enable_fast_printing=${enableval}, enable_fast_printing="yes")
if test "${enable_fast_printing}" = "yes"; then
    AC_DEFINE([FAST_PRINTING], [1], [Define if fast printing is preferred.])
fi


AM_MISSING_PROG([HELP2MAN], [help2man], ["${missing_dir}"])

## output
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_OUTPUT
