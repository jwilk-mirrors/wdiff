# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

_autoreconf = $(srcdir)/autogen.sh

config_h_header = ("wdiff\.h"|<config\.h>)
gnulib_dir = .

INDENT_STYLE = -gnu -ppi1 -ut
INDENT_SOURCES = src/*.c src/*.h

VC_LIST_ALWAYS_EXCLUDE_REGEX = ^(lib|m4)/
VC_LIST_EXCEPT_sc_GPL_version = po/.*\.po
VC_LIST_EXCEPT_sc_makefile_path_separator_check = po/Makefile\.in\.in
VC_LIST_EXCEPT_sc_makefile_TAB_only_indentation = po/Makefile\.in\.in
VC_LIST_EXCEPT_sc_po_check = lib/.*\.c
VC_LIST_EXCEPT_sc_prohibit_empty_lines_at_EOF = ^(ABOUT-NLS)$$
VC_LIST_EXCEPT_sc_prohibit_always-defined_macros = ^build-aux/
VC_LIST_EXCEPT_sc_indent = ^build-aux/

local-checks-to-skip += sc_program_name
local-checks-to-skip += sc_prohibit_atoi_atof
local-checks-to-skip += sc_prohibit_strcmp
local-checks-to-skip += sc_useless_cpp_parens

# Stuff maintained by maint.mk:
old_NEWS_hash = d41d8cd98f00b204e9800998ecf8427e
