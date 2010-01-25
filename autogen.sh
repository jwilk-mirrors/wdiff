#!/bin/sh

aclocal -I m4
autoconf
autoheader
touch ChangeLog
automake --gnu --add-missing
rm ChangeLog
