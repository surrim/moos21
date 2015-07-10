#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([moos], [2.1.1], [surrim@arcor.de])
AC_CONFIG_SRCDIR([src/base.cpp])
AM_INIT_AUTOMAKE
#AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([socket])

AC_CONFIG_FILES([Makefile
                 language/Makefile
                 src/Makefile])
AX_CXX_COMPILE_STDCXX_11([noext])
AX_CHECK_ZLIB
AM_OPTIONS_WXCONFIG
AM_PATH_WXCONFIG([2.8.0], [wxWin=1])
  if test "$wxWin" != 1 || test "$wx_ver_ok" != "yes"; then
    AC_MSG_ERROR([
      wxWidgets must be installed on your system.

      Please check that wx-config is in path, the directory
      where wxWidgets libraries are installed (returned by
      'wx-config --libs' or 'wx-config --static --libs' command)
      is in LD_LIBRARY_PATH or equivalent variable and
      wxWidgets version is $reqwx or above.
    ])
  fi
CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
LIBS="$LIBS $WX_LIBS"

AC_OUTPUT
