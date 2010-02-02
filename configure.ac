## Copying and distribution of this file, with or without modification,
## are permitted in any medium without royalty provided the copyright
## notice and this notice are preserved.  This file is offered as-is,
## without any warranty.

AC_INIT([GNU wdiff], [0.5.95], [wdiff-bugs@gnu.org])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE

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

# Handling of experimental applications
AC_ARG_ENABLE([experimental],
 AC_HELP_STRING([--enable-experimental],
  [enable experimental applications (mdiff, unify, wdiff2)]),
 [AS_CASE([$enableval],
  [yes], [EXPERIMENTAL="mdiff unify wdiff2"],
  [no],  [EXPERIMENTAL=""],
  [EXPERIMENTAL=$enableval])],
 [EXPERIMENTAL=""])
set dummy $EXPERIMENTAL; shift
EXPERIMENTAL_PROGRAMS=""
EXPERIMENTAL_MANS=""
for experimental_i
do
AS_CASE([$experimental_i],
  [mdiff|unify|wdiff2],
  [EXPERIMENTAL_PROGRAMS="$EXPERIMENTAL_PROGRAMS $experimental_i\$(EXEEXT)"
EXPERIMENTAL_MANS="$EXPERIMENTAL_MANS $experimental_i.1"],
  [AC_MSG_ERROR([unknown experimental $experimental_i])])
done
AC_MSG_NOTICE([Enabled experimental applications: $EXPERIMENTAL])
AC_SUBST([EXPERIMENTAL])
AC_SUBST([EXPERIMENTAL_PROGRAMS])
AC_SUBST([EXPERIMENTAL_MANS])
# Done handling experimental applications

AC_CONFIG_FILES([
 Makefile
 doc/Makefile
 lib/Makefile
 lib/po/Makefile.in
 man/Makefile
 po/Makefile.in
 src/Makefile
])
AC_OUTPUT
