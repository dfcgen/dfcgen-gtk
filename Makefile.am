## -*- mode: makefile-automake; coding: utf-8 -*-
##
## Copyright (C) 2006-2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
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


# make dox
dox:
	cd doc && $(MAKE) $(AM_MAKEFLAGS) dox

.PHONY: dox


# Maintainer rule for building the Debian package.
# NOTE: Generates the changelog starting with release 0.4-1.
dist_debian_dir="dist/$(distdir)"

dist-debian: dist
	git log --pretty --numstat --summary --no-merges --since=29/10/2016 | git2cl > ChangeLog.$(shell date '+%g%m%d')
	cp -f -t dist $(distdir).tar.gz
	cd dist && gunzip -c $(distdir).tar.gz | tar xf -
	cp -Rf dist/debian $(dist_debian_dir)
	cd $(dist_debian_dir) && dpkg-buildpackage -rfakeroot
	@echo '--------------------------------------------------------------------'
	@dpkg-parsechangelog -l./dist/debian/changelog
	@echo '--------------------------------------------------------------------'
	-rm -Rf $(dist_debian_dir)


# maintainer rule for "package" build on MinGW
if BUILD_OS_MINGW32

GTK_VERSION = $(shell $(PKG_CONFIG) --modversion "gtk+-2.0" | \
                    sed -n 's/^\([0-9]\+\)\.\([0-9]\+\)\(\.[0-9]*\)\?/\1\2/p')
dist_win32_dir = ./dist/$(PACKAGE)$(GTK_VERSION)-$(VERSION)

# all GTK/GDK DLLs needed by dfcgen-gtk (heuristically determined)
GTK_DLL_COMMON = libgio-2.0-0.dll libatk-1.0-0.dll libcairo-2.dll libgdk_pixbuf-2.0-0.dll \
         libgdk-win32-2.0-0.dll libglib-2.0-0.dll libgmodule-2.0-0.dll \
         libgobject-2.0-0.dll libgsl-0.dll libgslcblas-0.dll \
         libgtk-win32-2.0-0.dll libiconv-2.dll libintl-8.dll \
         libpango-1.0-0.dll libpangocairo-1.0-0.dll libpangowin32-1.0-0.dll \
         libpng*.dll libgthread-2.0-0.dll libpangoft2-1.0-0.dll zlib1.dll

GTK212_DLL_ADDONS = intl.dll libssp-0.dll
GTK220_DLL_ADDONS = freetype6.dll libexpat-1.dll libfontconfig-1.dll

GTK_DLL_LIST = $(GTK_DLL_COMMON) $(GTK212_DLL_ADDONS) $(GTK220_DLL_ADDONS)


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
	  echo -e "\tPackaging $(dist_win32_dir)/$$f"; \
	  cp -t $(dist_win32_dir) /mingw/bin/$$f 2>/dev/null; \
	  true; \
	done
	@echo
	@echo '--------------------------------------------------------------------'
	@echo "Win32 package in $(dist_win32_dir) ready for upload"
	@echo '--------------------------------------------------------------------'

endif



maintainer-clean-local:
	-rm -Rf $(dist_debiandir)



.PHONY: dist-debian dist-win32
