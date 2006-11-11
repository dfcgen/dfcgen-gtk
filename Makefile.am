## Process this file with automake to produce Makefile.in
## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

SUBDIRS = po include src pixmaps data doc

dfcgen_gtkdocdir = ${prefix}/share/gtk-doc/dfcgen-gtk
dfcgen_gtkdoc_DATA =

EXTRA_DIST = $(dfcgen_gtkdoc_DATA)



# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

