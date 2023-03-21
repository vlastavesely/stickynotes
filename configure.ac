#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([stickynotes], [1.0], [vlastavesely@protonmail.ch])
AC_CONFIG_SRCDIR([application.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

# Checks for libraries.
PKG_CHECK_MODULES(GTK, gtk+-3.0)
PKG_CHECK_MODULES(GSV, gtksourceview-3.0)
PKG_CHECK_MODULES(INDICATOR, ayatana-appindicator3-0.1)
PKG_CHECK_MODULES(X11, x11)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([strdup])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
