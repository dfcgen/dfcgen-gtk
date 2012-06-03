## -*- Makefile -*-
##
## Copyright (C) 2006, 2011-2012 Ralf Hoppe <ralf.hoppe@ieee.org>
##
## Version:  $Id$
##
## Process this file with automake to produce Makefile.in
##

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = po include src data doc dist

doc_DATA = \
	README\
	COPYING\
	ChangeLog\
	INSTALL


EXTRA_DIST = $(doc_DATA)


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)


# Maintainer rule for building a Debian package
dist_debian_tmpdir="dist/$(distdir)"

dist-debian: dist
	mkdir -p dist
	mv -f $(distdir).* dist
	cd dist && gunzip -c $(distdir).tar.gz | tar xf -
	cp -Rf dist/debian $(dist_debian_tmpdir)
	rm -Rf $(dist_debian_tmpdir)/debian/.svn
	cd $(dist_debian_tmpdir) && dpkg-buildpackage -rfakeroot
	@echo '--------------------------------------------------------------------'
	@dpkg-parsechangelog -l./dist/debian/changelog
	@echo '--------------------------------------------------------------------'
	-rm -Rf $(dist_debian_tmpdir)


maintainer-clean-local:
	-rm -Rf $(dist_debian_tmpdir)


.PHONY: dist-debian
