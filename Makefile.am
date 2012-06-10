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
dist_debian_dir="dist/$(distdir)"

dist-debian: dist
	cp -f -t dist $(distdir).tar.gz
	cd dist && gunzip -c $(distdir).tar.gz | tar xf -
	cp -Rf dist/debian $(dist_debian_dir)
	rm -Rf $(dist_debian_dir)/debian/.svn
	cd $(dist_debian_dir) && dpkg-buildpackage -rfakeroot
	@echo '--------------------------------------------------------------------'
	@dpkg-parsechangelog -l./dist/debian/changelog
	@echo '--------------------------------------------------------------------'
	-rm -Rf $(dist_debian_dir)


# maintainer rule for "package" build on MinGW
if BUILD_OS_MINGW32

GTK_VERSION=$(shell $(PKG_CONFIG) --modversion "gtk+-2.0" | \
                    sed -n 's/^\([0-9]\+\)\.\([0-9]\+\)\(\.[0-9]*\)\?/\1\2/p')
dist_win32_dir = ./dist/$(PACKAGE)$(GTK_VERSION)-$(VERSION)

# all GTK/GDK DLLs needed by dfcgen-gtk (heuristically determined)
GTK_DLL_LIST = intl.dll libatk-1.0-0.dll libcairo-2.dll libgdk_pixbuf-2.0-0.dll \
         libgdk-win32-2.0-0.dll libglib-2.0-0.dll libgmodule-2.0-0.dll \
         libgobject-2.0-0.dll libgsl-0.dll libgslcblas-0.dll \
         libgtk-win32-2.0-0.dll libiconv-2.dll libintl-8.dll \
         libpango-1.0-0.dll libpangocairo-1.0-0.dll libpangowin32-1.0-0.dll \
         libpng12-0.dll libssp-0.dll

dist-win32: all
	@echo "Building Win32 distribution in $(dist_win32_dir) ..."
	rm -fR $(dist_win32_dir)
	mkdir $(dist_win32_dir)
	cp ./src/$(PACKAGE)$(EXEEXT) $(dist_win32_dir)
	mkdir $(dist_win32_dir)/share
	cp -R ./data/pixmaps $(dist_win32_dir)/share && rm -fR $(dist_win32_dir)/share/pixmaps/Makefile* $(dist_win32_dir)/share/pixmaps/.svn
	cp -R ./data/filters $(dist_win32_dir)/share && rm -fR $(dist_win32_dir)/share/filters/Makefile* $(dist_win32_dir)/share/filters/.svn
	cp -R ./data/templates $(dist_win32_dir)/share && rm -fR $(dist_win32_dir)/share/templates/Makefile* $(dist_win32_dir)/share/templates/.svn
	mkdir $(dist_win32_dir)/share/locale
	cp -R /mingw/share/locale/de $(dist_win32_dir)/share/locale
	cp -R ./$(POSUB)/de.gmo $(dist_win32_dir)/share/locale/de/LC_MESSAGES/dfcgen-gtk.mo
	cp -R /mingw/etc $(dist_win32_dir)
	mkdir $(dist_win32_dir)/lib
	cp -R /mingw/lib/gtk-2.0 $(dist_win32_dir)/lib && rm -fR $(dist_win32_dir)/lib/gtk-2.0/include
	for f in $(GTK_DLL_LIST); do \
	  cp -t $(dist_win32_dir) /mingw/bin/$$f; \
	done
	@echo '--------------------------------------------------------------------'
	@echo "Win32 distribution in $(dist_win32_dir) ready for upload"
	@echo '--------------------------------------------------------------------'

endif



maintainer-clean-local:
	-rm -Rf $(dist_debiandir)



.PHONY: dist-debian dist-win32
