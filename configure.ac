dnl -*- Autoconf -*-
dnl
dnl Copyright (C) 2006, 2011-2012 Ralf Hoppe <ralf.hoppe@ieee.org>
dnl
dnl Version:  $Id$
dnl
dnl Process this file with autoconf to produce a configure script.
dnl
dnl Note: This file requires tool and library versions, which are provided
dnl       first by Ubuntu 8.04 (Hardy).
dnl

dnl Autoconf requirements
AC_PREREQ(2.65)

dnl AC_INIT() defines: PACKAGE_NAME, PACKAGE_TARNAME, PACKAGE_VERSION,
dnl                    PACKAGE_STRING, PACKAGE_BUGREPORT, PACKAGE_URL
 
AC_INIT([Digital Filter Coefficients Generator (GTK+)], [0.4],
        [ralf.hoppe@ieee.org],
        [dfcgen-gtk],
        [http://www.dfcgen.de])

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
                 doc/Makefile
                 dist/Makefile
                 dist/debian/Makefile
                 dist/win32/Makefile])


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


# -g  => include debug info
# -pg => include code for gprof (GNU profiler)
# -O0 => no optimization
CFLAGS_DEBUG="-g -pg -O0 -fstack-protector-all -UNDEBUG \
              -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"

CFLAGS_NDEBUG="-O3 -funroll-loops -DNDEBUG -DG_DISABLE_ASSERT"



dnl Set CFLAGS before calling AC_PROG_CC, else default values are set. For
dnl target specific flags use AM_CFLAGS in Makefile.am. Notice that CFLAGS
dnl overwrites AM_CFLAGS (caused by order of evaluation in Makefile.am), so
dnl use AM_CFLAGS only to specify additional flags.
CFLAGS="$CFLAGS $CFLAGS_LANG $CFLAGS_WARN"

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


dnl Additional Substitutions and Defines
AC_SUBST(PACKAGE_AUTHOR, ["Ralf Hoppe"])
AC_SUBST(COPYRIGHT_HOLDER, ["Copyright (C) 2006, 2011-2012 $PACKAGE_AUTHOR"])
AC_DEFINE_UNQUOTED(PACKAGE_AUTHOR, ["$PACKAGE_AUTHOR"], [package author])
AC_DEFINE_UNQUOTED(PACKAGE_COPYRIGHT, ["$COPYRIGHT_HOLDER"], [copyright holder])
AC_DEFINE(PACKAGE_ICON, ["dfcgen.png"], [package icon])

dnl some installation directories
pkgpixmapsdir='$(pkgdatadir)/pixmaps'
pkgfiltersdir='$(pkgdatadir)/filters'
pkgtemplatesdir='$(pkgdatadir)/templates'

AC_SUBST(pkgpixmapsdir)
AC_SUBST(pkgfiltersdir)
AC_SUBST(pkgtemplatesdir)



dnl Checks for programs (gcc)
AC_PROG_CC


dnl Define _GNU_SOURCE to enable header file extensions on GNU systems (macro
dnl AC_GNU_SOURCE is obsolete). Notice the ordering: after ‘AC_PROG_CC’ but
dnl before ‘AM_GNU_GETTEXT’, most likely very close to the ‘AC_PROG_CC’
dnl invocation.
dnl
dnl This macro is available since Autoconf 2.60 (Ubuntu 8.04 distributes
dnl Autoconf 2.61)
AC_USE_SYSTEM_EXTENSIONS


dnl GNU gettext
dnl
dnl Create a GETTEXT_PACKAGE make variable and define (config.h) with the same
dnl name as the package (see AC_INIT for package name declaration). The define
dnl is used in main.c for gettext domain binding.
GETTEXT_PACKAGE=AC_PACKAGE_TARNAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

dnl Instead of AM_GNU_GETTEXT also AM_GLIB_GNU_GETTEXT might be used, because
dnl (from the man page of glib-gettextize): "glib-gettextize differs from
dnl gettextize in that it doesn't create an intl/ subdirectory and doesn't
dnl modify po/ChangeLog". Note, that AM_GNU_GETTEXT([external]) has the same
dnl effect - it disables any intl/ subdirectory handling. This seems
dnl appropriate because libintl should be available on all GNU Linux systems.
dnl For details see the explanations on AM_GNU_GETTEXT() in the gettext manual.
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.17)


dnl Check for MinGW environment (do this after AC_PROG_CC)
dnl install from http://sourceforge.net/projects/mingw/files/Installer/mingw-get-inst
case "$host" in
  *-mingw32*)
     mingw_build=yes
     AC_CHECK_TOOL(WINDRES, windres)
    ;;
  *)
    mingw_build=no
    ;;
esac

AC_MSG_CHECKING([for MinGW])
AC_MSG_RESULT([$mingw_build])
AM_CONDITIONAL(MINGW_BUILD, test x$mingw_build = xyes)


dnl Check for libraries required by GSL, finally going into LIBS and LDFLAGS:
dnl http://www.gnu.org/software/gsl/manual/html_node/Autoconf-Macros.html
dnl
dnl Note: Commented out because all work done by PKG_CHECK_MODULES()
dnl
dnl AC_CHECK_LIB([m],[main])
dnl AC_CHECK_LIB([gslcblas],[main])
dnl AC_CHECK_LIB([gsl],[main])


dnl pkg-config library checks (automated -I, -l/L switches generation and
dnl PKGCONF_CFLAGS, PKGCONF_LIBS variable creation)
dnl
dnl Note: The version numbers used here should match the numbers declared in
dnl       Debian control file "./dist/debian/control".
PKG_CHECK_MODULES(PKGCONF, [gdk-2.0 gtk+-2.0 >= 2.12 gsl >= 1.10])



dnl Checks for header files
AC_HEADER_STDC
AC_CHECK_HEADER([gsl/gsl_math.h],
                [AC_DEFINE(HAVE_GSL_H, 1, [Define to 1 if you have GNU Scientific Library (GSL) header files])],
                [AC_MSG_ERROR([You must have installed GNU Scientific Library (GSL) header files])])



dnl Checks for typedefs and structures


dnl Checks for compiler characteristics.
AC_C_CONST
AC_C_RESTRICT
AC_C_INLINE


dnl Checks for library functions


dnl AC_CHECK_FUNCS() uses shell variables LIBS and LDFLAGS for the link in
dnl sub-function AC_LINK_IFELSE(). Both variables normally are provided by
dnl AC_CHECK_LIB() and friends. Because PKG_CHECK_MODULES() from pkg-config
dnl is used here, AC_CHECK_LIB() is not used and so LIBS probably is empty,
dnl which finally result in an always failing test here.
LIBS="$PKGCONF_LIBS $LIBS"

AC_CHECK_FUNCS(hypot)
AC_CHECK_FUNCS(trunc)
AC_CHECK_FUNCS(round)
AC_CHECK_FUNCS(exp10)
AC_CHECK_FUNCS(pow10)

AC_MSG_NOTICE([all functions checked with LIBS = '$LIBS'])


dnl Checks for system services


AC_OUTPUT

