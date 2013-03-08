## Copying and distribution of this file, with or without modification,
## are permitted in any medium without royalty provided the copyright
## notice and this notice are preserved.  This file is offered as-is,
## without any warranty.

SUBDIRS = lib po src doc man
ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST =

# --------- #
# ChangeLog #
# --------- #

EXTRA_DIST += ChangeLog
ChangeLog:
	LC_ALL=en_US.utf8 \
	bzr log --gnu-changelog --include-merged --omit-merges > $@.tmp
	cat $@.old >> $@.tmp
	mv $@.tmp $@

# -------------- #
# helper scripts #
# -------------- #

EXTRA_DIST += $(top_srcdir)/build-aux/msgitm.pm
EXTRA_DIST += $(top_srcdir)/build-aux/print-po.pl

## ----------- ##
## Test suite. ##
## ----------- ##
# (copied from Makefile.am in libtool CVS 2007-07-11)

# Using `cd' in backquotes may print the directory name, use this instead:
lt__cd		= CDPATH="$${ZSH_VERSION+.}$(PATH_SEPARATOR)" && cd

# The testsuite files are evaluated in the order given here.
TESTSUITE	= tests/testsuite
TESTSUITE_AT	= tests/testsuite.at \
		  tests/cluster.at \
		  tests/wdiff.at

EXTRA_DIST     += $(srcdir)/$(TESTSUITE) $(TESTSUITE_AT) $(srcdir)/tests/package.m4 m4/gnulib-cache.m4

# Be sure to reexport important environment variables:
TESTS_ENVIRONMENT = MAKE="$(MAKE)" CC="$(CC)" CFLAGS="$(CFLAGS)" \
	CPP="$(CPP)" CPPFLAGS="$(CPPFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" \
	LIBS="$(LIBS)" LN_S="$(LN_S)" NM="$(NM)" RANLIB="$(RANLIB)" \
	OBJEXT="$(OBJEXT)" EXEEXT="$(EXEEXT)" \
	SHELL="$(SHELL)" CONFIG_SHELL="$(SHELL)" \
	CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" CXXCPP="$(CXXCPP)" \
	F77="$(F77)" FFLAGS="$(FFLAGS)" \
	FC="$(FC)" FCFLAGS="$(FCFLAGS)" \
	GCJ="$(GCJ)" GCJFLAGS="$(GCJFLAGS)"

# Use `$(srcdir)' for the benefit of non-GNU makes: this is
# how `testsuite' appears in our dependencies.
$(srcdir)/$(TESTSUITE): $(srcdir)/tests/package.m4 $(TESTSUITE_AT) Makefile.am
	cd $(srcdir)/tests && \
	$(AUTOTEST) `echo $(TESTSUITE_AT) | sed 's,tests/,,g'` -o testsuite.tmp && \
	mv -f testsuite.tmp testsuite

$(srcdir)/tests/package.m4: $(srcdir)/configure.ac
	{ \
	  echo '# Signature of the current package.'; \
	  echo 'm4_define([AT_PACKAGE_NAME],      [$(PACKAGE_NAME)])'; \
	  echo 'm4_define([AT_PACKAGE_TARNAME],   [$(PACKAGE_TARNAME)])'; \
	  echo 'm4_define([AT_PACKAGE_VERSION],   [$(PACKAGE_VERSION)])'; \
	  echo 'm4_define([AT_PACKAGE_STRING],    [$(PACKAGE_STRING)])'; \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT], [$(PACKAGE_BUGREPORT)])'; \
	} > $(srcdir)/tests/package.m4

tests/atconfig: $(top_builddir)/config.status
	$(SHELL) ./config.status tests/atconfig

DISTCLEANFILES = tests/atconfig

CD_TESTDIR	= abs_srcdir=`$(lt__cd) $(srcdir) && pwd`; cd tests

# Hook the test suite into the check rule
check-local: tests/atconfig $(srcdir)/$(TESTSUITE) $(bin_SCRIPTS)
	$(CD_TESTDIR); \
	CONFIG_SHELL="$(SHELL)" $(SHELL) $$abs_srcdir/$(TESTSUITE) \
	  $(TESTSUITEFLAGS) AUTOTEST_PATH="src"

# Run the test suite on the *installed* tree.
installcheck-local:
	$(CD_TESTDIR); \
	CONFIG_SHELL="$(SHELL)" $(SHELL) $$abs_srcdir/$(TESTSUITE) \
	  $(TESTSUITEFLAGS) AUTOTEST_PATH="$(exec_prefix)/bin"

# We need to remove any file droppings left behind by testsuite
clean-local:
	-$(CD_TESTDIR); \
	test -f $$abs_srcdir/$(TESTSUITE) && \
	    $(SHELL) $$abs_srcdir/$(TESTSUITE) --clean
