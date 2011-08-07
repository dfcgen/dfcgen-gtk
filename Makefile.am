## -*- Makefile -*-
##
## Copyright (c) 2006, 2011 Ralf Hoppe <ralf.hoppe@ieee.org>
##
## Version:  $Header$
##
## Process this file with automake to produce Makefile.in
##

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = po include src pixmaps data doc

doc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

EXTRA_DIST = config.rpath  $(doc_DATA)


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)
