## -*- Makefile -*-
##
## Copyright (c) 2006, 2011-2012 Ralf Hoppe <ralf.hoppe@ieee.org>
##
## Version:  $Id$
##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR="\"$(localedir)\"" \
	-DPACKAGE_DATA_DIR="\"$(pkgdatadir)\"" \
	-I"$(top_srcdir)/include" \
	@PACKAGE_CFLAGS@


# Additional CFLAGS
# AM_CFLAGS =

# Additional LDFLAGS
# AM_LDFLAGS =


bin_PROGRAMS = dfcgen-gtk

# Use PACKAGE_LIBS from PKG_CHECK_MODULES (pkg-config)
dfcgen_gtk_LDADD = @PACKAGE_LIBS@


dfcgen_gtk_SOURCES = \
	support.c \
	main.c \
	designDlg.c \
	mainDlg.c \
	dfcProject.c \
	filterResponse.c \
	filterSupport.c \
	mathPoly.c \
	stdIirFilter.c \
	helpDlg.c \
	responseWin.c \
	mathMisc.c \
	mathFuncs.c \
	cairoPlot.c \
	fileDlg.c \
	cfgSettings.c \
	responseDlg.c \
	editDlg.c \
	stdIirDesignDlg.c \
	miscDesignDlg.c \
	projectFile.c \
	miscFilter.c \
	dialogSupport.c \
	responsePlot.c \
	linFirDesignDlg.c \
	linFirFilter.c \
	rootsPlot.c \
	filterPrint.c
