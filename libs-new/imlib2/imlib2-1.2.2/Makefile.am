## Process this file with automake to produce Makefile.in

SUBDIRS = src data doc

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 config.guess \
                       config.h.in config.sub configure install-sh \
		       ltconfig ltmain.sh missing mkinstalldirs \
		       stamp-h.in build-stamp configure-stamp depcomp \
		       imlib2_docs.tar.gz imlib2.c \
		       README \
		       imlib2.pc \
		       imlib2.spec \
		       imlib2.oe imlib2-native.oe \
		       imlib2.bb imlib2Xnative.bb \
		       debian/changelog

bin_SCRIPTS = imlib2-config

EXTRA_DIST = AUTHORS COPYING COPYING-PLAIN gendoc \
             Doxyfile \
	     README.in README \
	     imlib2.pc.in \
	     imlib2.spec.in imlib2.spec \
	     imlib2.oe.in imlib2.oe imlib2-native.oe.in imlib2-native.oe \
	     imlib2.bb.in imlib2.bb imlib2Xnative.bb.in imlib2Xnative.bb \
	     debian/changelog \
	     debian/changelog.in \
	     debian/control \
	     debian/copyright \
	     debian/imlib2-config.1 \
	     debian/imlib2-demo.install \
	     debian/imlib2-test.install \
	     debian/libimlib2-dev.doc-base \
	     debian/libimlib2-dev.install \
	     debian/libimlib2.install \
	     debian/rules

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = imlib2.pc
