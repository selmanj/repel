#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT(pel, 0.1, selman at eecs.oregonstate.edu)
AC_CONFIG_SRCDIR([src/])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_ARG_WITH([boost-include-path],
            [AS_HELP_STRING([--with-boost-include-path],
                            [location of the boost headers, defaults to /usr/local/boost_1_46_1])],
            [BOOST_CFLAGS="-I$withval"],
            [BOOST_CFLAGS="-I/usr/local/boost_1_46_1"])
AC_SUBST([BOOST_CFLAGS])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
