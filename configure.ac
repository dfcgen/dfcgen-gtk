dnl -*- Autoconf -*-
dnl
dnl Copyright (C) 2006, 2011-2012 Ralf Hoppe <ralf.hoppe@ieee.org>
dnl
dnl Version:  $Id$
dnl
dnl Process this file with autoconf to produce a configure script.
dnl
dnl Note: This file uses pkg-config and associated M4 macro PKG_CHECK_MODULES().
dnl       So it runs only on systems, where pkg-config is available, mostly all
dnl       Linux distributions. On Ubuntu 10.04 LTS (Lucid Lynx) the version of
dnl       Autoconf is 2.65 (AC_USE_SYSTEM_EXTENSIONS was introduced in v2.60).

AC_PREREQ(2.65)

AC_INIT([dfcgen-gtk],[0.3],[ralf.hoppe@ieee.org])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_FILES([Makefile
                 po/Makefile.in
                 include/Makefile
                 src/Makefile
                 data/Makefile
                 data/pixmaps/Makefile
                 data/filters/Makefile
                 data/templates/Makefile
                 doc/Makefile])

dnl Automake options: we are not strictly GNU compatible (foreign),
dnl because the following files are missing: NEWS, AUTHORS, ABOUT-NLS
AM_INIT_AUTOMAKE([-Wall foreign])
AM_MAINTAINER_MODE


CFLAGS_LANG="-std=c99"
CFLAGS_WARN="-Wall -Wundef -Wshadow \
-Wdeclaration-after-statement \
-Wpointer-arith -Wcast-align \
-Wmissing-prototypes -Wmissing-declarations \
-Wredundant-decls"

# The most conservative approach is to use the i568 (pentium) architecture
# specification. This architecture is compatible with both CPU vendors, Intel
# and AMD (see the GCC manual for other alternatives).
CFLAGS_MACHINE="-march=i586"

# -g  => include debug info
# -pg => include code for gprof (GNU profiler)
# -O0 => no optimization
CFLAGS_DEBUG="-g -pg -O0 -fstack-protector-all -UNDEBUG"
# CFLAGS_DEBUG="-O3 -funroll-loops -fstack-protector-all -UNDEBUG"
CFLAGS_NDEBUG="-O3 -funroll-loops -DNDEBUG -DG_DISABLE_ASSERT"



dnl Set CFLAGS before calling AC_PROG_CC, else default values are set. For
dnl target specific flags use AM_CFLAGS in Makefile.am. Notice that CFLAGS
dnl overwrites AM_CFLAGS (caused by order of evaluation in Makefile.am), so
dnl use AM_CFLAGS only to specify additional flags.
CFLAGS="$CFLAGS $CFLAGS_MACHINE $CFLAGS_LANG $CFLAGS_WARN"

AC_MSG_CHECKING(whether to enable debugging)
AC_ARG_ENABLE([debug],
              [AS_HELP_STRING([--enable-debug], [enable debugging (default=no)])],
              [
                  if test "x$enable_debug" = "xyes"; then
                     AC_DEFINE([DEBUG], [1],
                               [Defined if '--enable-debug' was passed to configure.])
                     CFLAGS="$CFLAGS $CFLAGS_DEBUG"
                  else
                     CFLAGS="$CFLAGS $CFLAGS_NDEBUG"
                  fi

                  AC_MSG_RESULT($enable_debug)
              ],
              [
                  CFLAGS="$CFLAGS $CFLAGS_NDEBUG"
                  AC_MSG_RESULT(no)
              ])


dnl Because Anjuta does not understand the third and fourth parameter of
dnl AC_INIT, the package URL is declared this way.
AC_DEFINE(PACKAGE_URL, ["http://www.dfcgen.de"], [package homepage])

dnl Additional Substitutions and Defines
AC_SUBST(PACKAGE_AUTHOR, ["Ralf Hoppe"])
AC_SUBST(COPYRIGHT_HOLDER, ["Copyright (C) 2006, 2011-2012 $PACKAGE_AUTHOR"])
AC_DEFINE_UNQUOTED(PACKAGE_AUTHOR, ["$PACKAGE_AUTHOR"], [package author])
AC_DEFINE_UNQUOTED(PACKAGE_COPYRIGHT, ["$COPYRIGHT_HOLDER"], [copyright holder])
AC_DEFINE(PACKAGE_ICON, ["dfcgen.png"], [package icon])


dnl Checks for programs (gcc)
dnl LT_INIT
AC_PROG_CC


dnl Define _GNU_SOURCE to enable header file extensions on GNU systems (macro
dnl AC_GNU_SOURCE is obsolete). Notice the ordering: after ‘AC_PROG_CC’ but
dnl before ‘AM_GNU_GETTEXT’, most likely very close to the ‘AC_PROG_CC’
dnl invocation.
AC_USE_SYSTEM_EXTENSIONS


dnl GNU gettext
dnl
dnl Create a GETTEXT_PACKAGE make variable and define (config.h) with the same
dnl name as the package (see AC_INIT for package name declaration). The define
dnl is used in main.c for gettext domain binding.
GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

dnl Instead of AM_GNU_GETTEXT also AM_GLIB_GNU_GETTEXT might be used, because
dnl (from the man page of glib-gettextize): "glib-gettextize differs from
dnl gettextize in that it doesn't create an intl/ subdirectory and doesn't
dnl modify po/ChangeLog".  Note, that AM_GNU_GETTEXT([external]) has the same
dnl effect - it disables any intl/ subdirectory handling. This seems
dnl appropriate because libintl should be available on all Linux systems. For
dnl details see the explanations on AM_GNU_GETTEXT() in the automake manual.
AM_GNU_GETTEXT([external])


dnl pkg-config library checks (automated -I, -l/L switches generation and
dnl PACKAGE_CFLAGS, PACKAGE_LIBS, PACKAGE_LDFLAGS variable creation)
dnl
dnl Note: The version numbers used here should match the numbers declared in
dnl       Debian control file "./dist/debian/control".
PKG_CHECK_MODULES(PACKAGE, [gtk+-2.0 >= 2.12 gdk-2.0 gsl >= 1.10])
AC_SUBST([PACKAGE_CFLAGS])
AC_SUBST([PACKAGE_LIBS])


dnl Checks for libraries
AC_CHECK_LIB(m,main)
AC_CHECK_LIB(gtk-x11-2.0,main)
AC_CHECK_LIB(glib-2.0,g_main_loop_run)
AC_CHECK_LIB(gslcblas,main)
AC_CHECK_LIB(gsl,main)


dnl Checks for header files
AC_HEADER_STDC
AC_CHECK_HEADER([gsl/gsl_math.h],
                [AC_DEFINE(HAVE_GSL_H, 1, [Define to 1 if you have GNU Scientific Library (GSL) header files])],
                [AC_MSG_ERROR([You must have installed GNU Scientific Library (GSL) header files])])


dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_RESTRICT
AC_C_INLINE


dnl Checks for library functions
AC_CHECK_FUNCS(hypot)
AC_CHECK_FUNCS(trunc)
AC_CHECK_FUNCS(round)
AC_CHECK_FUNCS(exp10)
AC_CHECK_FUNCS(pow10)


AC_OUTPUT

