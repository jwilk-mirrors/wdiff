#!/bin/bash

# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

rm -r lib/* m4/* build-aux/*.h
for i in build-aux/*; do
    [[ -L $i ]] && continue
    case "$i" in
        *.sh|*.pm|*.pl)
            ;;
        *)
            rm $i
            ;;
    esac
done
bzr revert m4/gnulib-cache.m4
autopoint --force
${GNULIB_TOOL:-gnulib-tool} --import "$@"
build-aux/regen-ignore.sh
bzr status
