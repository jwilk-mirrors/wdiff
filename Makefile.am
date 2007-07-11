SUBDIRS = lib po src doc man
ACLOCAL_AMFLAGS = -I m4


## ----------- ##
## Test suite. ##
## ----------- ##
# (copied from Makefile.am in libtool CVS 2007-07-11)

edit = sed \
	-e 's,@EGREP\@,$(EGREP),g' \
	-e 's,@FGREP\@,$(FGREP),g' \
	-e 's,@GREP\@,$(GREP),g' \
	-e 's,@LN_S\@,$(LN_S),g' \
	-e 's,@MACRO_VERSION\@,$(VERSION),g' \
	-e 's,@PACKAGE\@,$(PACKAGE),g' \
	-e 's,@PACKAGE_BUGREPORT\@,$(PACKAGE_BUGREPORT),g' \
	-e 's,@PACKAGE_NAME\@,$(PACKAGE),g' \
	-e 's,@PACKAGE_STRING\@,$(PACKAGE) $(VERSION),g' \
	-e 's,@PACKAGE_TARNAME\@,$(PACKAGE),g' \
	-e 's,@PACKAGE_VERSION\@,$(VERSION),g' \
	-e 's,@SED\@,$(SED),g' \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@aclocaldir\@,$(aclocaldir),g' \
	-e 's,@datadir\@,$(datadir),g' \
	-e 's,@pkgdatadir\@,$(pkgdatadir),g' \
	-e 's,@host_triplet\@,$(host_triplet),g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e "s,@configure_input\@,Generated from $$input; do not edit by hand,g"

# Using `cd' in backquotes may print the directory name, use this instead:
lt__cd		= CDPATH="$${ZSH_VERSION+.}$(PATH_SEPARATOR)" && cd

# The testsuite files are evaluated in the order given here.
TESTSUITE	= tests/testsuite
TESTSUITE_AT	= tests/testsuite.at \
		  tests/cluster.at \
		  tests/wdiff.at

EXTRA_DIST     = $(srcdir)/$(TESTSUITE) $(TESTSUITE_AT) $(srcdir)/tests/package.m4

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
	  echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])'; \
	  echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])'; \
	  echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])'; \
	  echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])'; \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} | $(edit) > $(srcdir)/tests/package.m4

tests/atconfig: $(top_builddir)/config.status
	$(SHELL) ./config.status tests/atconfig

DISTCLEANFILES = tests/atconfig

CD_TESTDIR	= abs_srcdir=`$(lt__cd) $(srcdir) && pwd`; cd tests

# Hook the test suite into the check rule
check-local: tests/atconfig $(srcdir)/$(TESTSUITE) $(bin_SCRIPTS)
	$(CD_TESTDIR); \
	CONFIG_SHELL="$(SHELL)" $(SHELL) $$abs_srcdir/$(TESTSUITE) \
	  $(TESTSUITEFLAGS)

# Run the test suite on the *installed* tree.
installcheck-local:
	$(CD_TESTDIR); \
	CONFIG_SHELL="$(SHELL)" $(SHELL) $$abs_srcdir/$(TESTSUITE) \
	  $(TESTSUITEFLAGS) AUTOTEST_PATH="$(exec_prefix)/bin"

# We need to remove any file droppings left behind by testsuite
clean-local: clean-local-legacy
	-$(CD_TESTDIR); \
	test -f $$abs_srcdir/$(TESTSUITE) && \
	    $(SHELL) $$abs_srcdir/$(TESTSUITE) --clean


# added by gettextize 0.16.1
EXTRA_DIST += config.rpath m4/ChangeLog
