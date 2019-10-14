## Makefile.am for libgdiplus/tests

LIBS = $(GDIPLUS_LIBS)

AM_CPPFLAGS =					\
	-I$(top_srcdir)				\
	-I$(top_builddir)/src		\
	-I$(top_srcdir)/src			\
	$(GDIPLUS_CFLAGS)

TEST_DEPS =						\
	$(top_builddir)/src/libgdiplus.la


LDADDS =					\
	$(top_builddir)/src/libgdiplus.la \
	-lm

noinst_PROGRAMS =

EXTRA_DIST =			\
	$(gtest_SOURCES) \
	AdobeRGB1998.icc \
	test-gsa.png \
	test-trns.png \
	test.bmp \
	test.emf \
	test.gif \
	test.ico \
	test.jpg \
	test.otf \
	test.png \
	test.tif \
	test.ttf \
	test.wmf \
	testhelpers.h \
	lsansuppressions.txt \
	$(NULL)

if HAVE_CMAKE

GTEST_BUILDDIR := $(top_builddir)/gtest_build
GTEST_LIB := $(GTEST_BUILDDIR)/googlemock/gtest/libgtest.a

$(GTEST_BUILDDIR):
	mkdir -p $(GTEST_BUILDDIR)

$(GTEST_BUILDDIR)/.stamp-gtest: | $(GTEST_BUILDDIR)
	cd $(GTEST_BUILDDIR) && $(CMAKE) . $(top_srcdir)/external/googletest && $(MAKE)
	touch $@

$(GTEST_LIB): $(GTEST_BUILDDIR)/.stamp-gtest

gtest_SOURCES =			\
	gtest-driver.cpp \
	gtest.cpp \
	testadjustablearrowcap.cpp \
	testbitmap.cpp \
	testbits.cpp \
	testbmpcodec.cpp \
	testbrush.cpp \
	testclip.cpp \
	testcodecs.cpp \
	testcustomlinecap.cpp \
	testemfcodec.cpp \
	testfont.cpp \
	testgeneral.cpp \
	testgifcodec.cpp \
	testgpimage.cpp \
	testgraphics.cpp \
	testgraphicsdraw.cpp \
	testgraphicsfill.cpp \
	testgraphicspath.cpp \
	testhatchbrush.cpp \
	testicocodec.cpp \
	testimageattributes.cpp \
	testlineargradientbrush.cpp \
	testmatrix.cpp \
	testmetafile.cpp \
	testpathgradientbrush.cpp \
	testpen.cpp \
	testpng.cpp \
	testpngcodec.cpp \
	testregion.cpp \
	testreversepath.cpp \
	testsolidbrush.cpp \
	teststringformat.cpp \
	testtexturebrush.cpp \
	testtext.cpp \
	testwmfcodec.cpp

if HAVE_LIBJPEG
gtest_SOURCES += testjpegcodec.cpp
endif HAVE_LIBJPEG

if HAVE_LIBTIFF
gtest_SOURCES += testtiffcodec.cpp
endif HAVE_LIBTIFF

gtest_DEPENDENCIES = $(TEST_DEPS) $(GTEST_LIB)
gtest_LDADD = $(LDADDS) $(GTEST_LIB)
gtest_CPPFLAGS = -pthread -I$(top_srcdir)/external/googletest/googletest/include -I$(top_srcdir)/src $(GDIPLUS_INCLUDES) -DGTEST

noinst_PROGRAMS += gtest

check: run-gtest

run-gtest: gtest
	./gtest --gtest_shuffle --gtest_output=xml:TestResult.xml

endif
