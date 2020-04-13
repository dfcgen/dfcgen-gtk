## -*- mode: makefile-automake -*-
##
## Copyright (c) 2006-2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
##

EXTRA_DIST =\
	dfcgen.sty\
	Doxyfile

override TEXINPUTS = .:$(abs_srcdir):
export TEXINPUTS

dox:
	doxygen

.PHONY: dox


maintainer-clean-local:
	-rm -Rf html/ latex/
