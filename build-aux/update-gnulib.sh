#!/bin/bash

# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

rm -r lib/*
for i in m4/*; do
    [[ -L $i ]] && continue
    case $i in
        m4/gnulib-cache.m4)
            ;;
        *)
            rm "$i"
            ;;
    esac
done
for i in build-aux/*; do
    [[ -L $i ]] && continue
    case $i in
        *.sh|*.pm|*.pl)
            ;;
        *)
            rm "$i"
            ;;
    esac
done
autopoint --force
${GNULIB_TOOL:-gnulib-tool} --import "$@"
build-aux/regen-ignore.sh
bzr status
