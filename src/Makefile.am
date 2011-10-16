## -*- Makefile -*-
##
## Copyright (c) 2006, 2011 Ralf Hoppe <ralf.hoppe@ieee.org>
##
## Version:  $Id$
##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_PIXMAPS_DIR=\""$(pkgdatadir)/pixmaps"\" \
	-I../include \
	@PACKAGE_CFLAGS@


# Additional CFLAGS
AM_CFLAGS = \
	 -Wall \
	 -Wuninitialized \
	 -Wmissing-prototypes


# Use PACKAGE_LIBS from PKG_CHECK_MODULES (pkg-config)
AM_LDFLAGS = @PACKAGE_LIBS@


bin_PROGRAMS = dfcgen-gtk

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
