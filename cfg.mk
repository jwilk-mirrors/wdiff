_autoreconf = $(srcdir)/autogen.sh

config_h_header = ("system\.h"|<config\.h>)

VC_LIST_ALWAYS_EXCLUDE_REGEX = ^(lib|m4)/
VC_LIST_EXCEPT_sc_GPL_version = po/.*\.po
VC_LIST_EXCEPT_sc_makefile_path_separator_check = po/Makefile\.in\.in
VC_LIST_EXCEPT_sc_makefile_TAB_only_indentation = po/Makefile\.in\.in
VC_LIST_EXCEPT_sc_po_check = lib/.*\.c

local-checks-to-skip += sc_program_name
local-checks-to-skip += sc_prohibit_atoi_atof
local-checks-to-skip += sc_prohibit_strcmp
local-checks-to-skip += sc_useless_cpp_parens

# Stuff maintained by maint.mk:
old_NEWS_hash = d41d8cd98f00b204e9800998ecf8427e
