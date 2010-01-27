#!/bin/bash

rm -r lib/* m4/*
for i in build-aux/*; do
    [[ -L $i ]] && continue
    case "$i" in
        *.sh)
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
