## -*- mode: makefile-automake -*-
##
## Copyright (C) 2006, 2011-2013, 2019 Ralf Hoppe <ralf.hoppe@dfcgen.de>
##

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	-DPACKAGE_PIXMAPS_DIR=\"$(pkgpixmapsdir)\" \
	-DPACKAGE_FILTERS_DIR=\"$(pkgfiltersdir)\" \
	-DPACKAGE_TEMPLATES_DIR=\"$(pkgtemplatesdir)\" \
	-I"$(top_srcdir)/include"


# Additional CFLAGS
AM_CFLAGS = @PKGCONF_CFLAGS@


# Additional LDFLAGS
#
# Note: There is no need to put in here PKGCONF_LIBS (from PKG_CHECK_MODULES(),
#       pkg-config), because configure.ac has put this variable into LIBS.
#
# AM_LDFLAGS = 


bin_PROGRAMS = dfcgen-gtk

# need for LIBINTL from AM_GNU_GETTEXT() - gettext.m4
dfcgen_gtk_LDADD = @LIBINTL@

# when building on MinGW: add resource file and Win32 compiler flags
if BUILD_OS_MINGW32
PACKAGE_RC_FILE = dfcgen-gtk.rc
dfcgen_gtk_LDADD += $(PACKAGE_RC_FILE).o
dfcgen_gtk_LDFLAGS = -mwindows
endif


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


# when building on MinGW: regard an additional rule on "make [all]", which
#                         executes the resource compiler
if BUILD_OS_MINGW32
all-local: $(PACKAGE_RC_FILE).o

$(PACKAGE_RC_FILE).o: ../dist/win32/$(PACKAGE_RC_FILE)
	$(WINDRES) -i ../dist/win32/$(PACKAGE_RC_FILE) --input-format=rc -o $(PACKAGE_RC_FILE).o -O coff

endif
