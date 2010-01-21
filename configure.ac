AC_INIT([GNU wdiff], [0.5.93], [wdiff-bugs@gnu.org])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror])

AC_CONFIG_HEADERS([config.h])

## ------------------------ ##
## Autotest initialisation. ##
## ------------------------ ##

AC_CONFIG_TESTDIR([tests])
AC_SUBST([AUTOM4TE], [${AUTOM4TE=autom4te}])
AC_SUBST([AUTOTEST], ['$(AUTOM4TE) --language=autotest'])


AC_PROG_CC
gl_EARLY
gl_INIT

# GNU help2man creates man pages from --help output; in many cases, this
# is sufficient, and obviates the need to maintain man pages separately.
# However, this means invoking executables, which we generally cannot do
# when cross-compiling, so we test to avoid that (the variable
# "cross_compiling" is set by AC_PROG_CC).
if test $cross_compiling = no; then
  AM_MISSING_PROG(HELP2MAN, help2man)
else
  HELP2MAN=:
fi

AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])

AC_CONFIG_FILES([
 Makefile
 doc/Makefile
 lib/Makefile
 man/Makefile
 po/Makefile.in
 src/Makefile
])
AC_OUTPUT
