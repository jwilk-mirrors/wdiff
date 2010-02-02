#!/bin/sh

# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

aclocal -I m4
autoconf
autoheader
touch ChangeLog
automake --gnu --add-missing
rm ChangeLog
