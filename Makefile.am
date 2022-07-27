## -*- mode: makefile-automake -*-
##
## Copyright (C) 2006-2022 Ralf Hoppe <ralf.hoppe@dfcgen.de>
##

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = po include src data doc dist

doc_DATA = \
	README\
	COPYING\
	ChangeLog\
	INSTALL


EXTRA_DIST = $(doc_DATA)
distdir_sub = dist


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)


# make dox
dox:
	cd doc && $(MAKE) $(AM_MAKEFLAGS) dox

.PHONY: dox


# get rid of this warning: "warning: shell ...  non-POSIX variable name"
AUTOMAKE_OPTIONS = -Wno-portability


# Maintainer rule for building a Debian package.
# NOTE: Generates the changelog starting with release 0.4-1.
distdir_debian = "$(distdir_sub)/$(distdir)"

dist-debian: dist
	git log --pretty --numstat --summary --no-merges --since=29/10/2016 | \
git2cl > "ChangeLog.$(shell date '+%g%m%d')"
	cp -f -t $(distdir_sub) $(distdir).tar.gz
	cd $(distdir_sub) && gunzip -c $(distdir).tar.gz | tar xf -
	cp -fR $(distdir_sub)/debian $(distdir_debian)
	cd $(distdir_debian) && dpkg-buildpackage -rfakeroot
	@echo '--------------------------------------------------------------------'
	@dpkg-parsechangelog -l./$(distdir_sub)/debian/changelog
	@echo '--------------------------------------------------------------------'
	-rm -Rf $(distdir_debian)


# Maintainer rule for Win64 build with MXE
if BUILD_OS_MINGW32

GTK_VERSION = $(shell $(PKG_CONFIG) --modversion 'gtk+-3.0' | \
sed -n 's/^\([0-9]\+\)\.\([0-9]\+\)\(\.[0-9]*\)\?/\1\2/p')

dist_win64 = "$(PACKAGE)$(GTK_VERSION)-$(VERSION)"
distdir_win64 = ./$(distdir_sub)/$(dist_win64)
dist_archive = $(dist_win64).zip

# MXE (M cross environment)
#
# 0. Background:
# - MXE Homepage: https://mxe.cc
# - MXE Sources: https://github.com/mxe/mxe
# - GTK 3 cross compile to Windows: https://gitlab.melroy.org/melroy/gtk-3-bundle-for-windows
# - Setting up GTK for Windows: https://www.gtk.org/docs/installations/windows/
#
# 1. Setup:
#    - download: git clone https://github.com/mxe/mxe.git
#    - prepare file mxe/settings.mk, then simply do: make -C mxe
#      or, build w/o mxe/settings.mk: cd mxe && make gtk3 gsl adwaita-icon-theme -j8 MXE_TARGETS='x86_64-w64-mingw32.static'
#    - install: chmod -R o+r,o+X mxe; sudo cp -R mxe /opt
#
# 2. Build:
#    - export PATH="/opt/mxe/usr/bin:$PATH"
#    - ./configure --host='x86_64-w64-mingw32.static'
#    - make clean dist-win64
#
mxe_host_platform = x86_64-w64-mingw32.static
mxe_win64_dir = /opt/mxe/usr/$(mxe_host_platform)
mxe_bin_dir = /opt/mxe/usr/bin

dist-win64: all
	@echo "Building Win64 distribution in $(distdir_win64) ..."
	rm "./$(distdir_sub)/$(dist_archive)"
	rm -fR $(distdir_win64)
	mkdir -p $(distdir_win64)
	mkdir -p $(distdir_win64)/share
	mkdir -p $(distdir_win64)/share/glib-2.0
	mkdir -p $(distdir_win64)/share/locale
	mkdir -p $(distdir_win64)/share/icons
	mkdir -p $(distdir_win64)/etc/gtk-3.0
	cp ./src/$(PACKAGE)$(EXEEXT) $(distdir_win64)
	cp -R ./data/pixmaps $(distdir_win64)/share && rm -fR $(distdir_win64)/share/pixmaps/Makefile*
	cp -R ./data/filters $(distdir_win64)/share && rm -fR $(distdir_win64)/share/filters/Makefile*
	cp -R ./data/templates $(distdir_win64)/share && rm -fR $(distdir_win64)/share/templates/Makefile*
	cp -R $(mxe_win64_dir)/share/locale/de $(distdir_win64)/share/locale
	cp -R ./$(POSUB)/de.gmo $(distdir_win64)/share/locale/de/LC_MESSAGES/dfcgen-gtk.mo
	cp -R $(mxe_win64_dir)/share/icons/Adwaita $(distdir_win64)/share/icons
	cp -R $(mxe_win64_dir)/share/glib-2.0/schemas $(distdir_win64)/share/glib-2.0
	glib-compile-schemas $(distdir_win64)/share/glib-2.0/schemas
	cp ./$(distdir_sub)/win64/settings.ini $(distdir_win64)/etc/gtk-3.0
	cd ./$(distdir_sub); zip -r $(dist_archive) $(dist_win64)
	@echo
	@echo '--------------------------------------------------------------------'
	@echo "Win64 archive $(dist_archive) ready for upload"
	@echo '--------------------------------------------------------------------'
	-rm -Rf $(distdir_win64)

endif



maintainer-clean-local:
	-rm -fR $(distdir_debian)
	-rm -fR $(distdir_win64)



.PHONY: dist-debian dist-win64
