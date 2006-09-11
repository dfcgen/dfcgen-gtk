## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = \
	$(PACKAGE_CFLAGS)\
	 -I../include

AM_CFLAGS =\
	 -DHAVE_CONFIG_H -DDEBUG\
	 -Wall

bin_PROGRAMS = dfcgen-gtk

dfcgen_gtk_SOURCES = \
	support.c\
	main.c\
	designDlg.c\
	mainDlg.c\
	dfcProject.c\
	filterResponse.c\
	filterSupport.c\
	mathPoly.c\
	stdIirFilter.c\
	helpDlg.c\
	responseWin.c\
	mathMisc.c\
	mathFuncs.c\
	cairoPlot.c\
	fileDlg.c\
	cfgSettings.c\
	responseDlg.c\
	editDlg.c\
	stdIirDesignDlg.c\
	miscDesignDlg.c\
	projectFile.c\
	miscFilter.c\
	dialogSupport.c\
	responsePlot.c

dfcgen_gtk_LDFLAGS = 

dfcgen_gtk_LDADD =  \
	$(PACKAGE_LIBS)

