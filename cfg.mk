_autoreconf = $(srcdir)/autogen.sh
VC_LIST_ALWAYS_EXCLUDE_REGEX = ^(lib|m4)/
config_h_header = ("system\.h"|<config\.h>)

local-checks-to-skip += sc_avoid_if_before_free
local-checks-to-skip += sc_cast_of_x_alloc_return_value
local-checks-to-skip += sc_program_name
local-checks-to-skip += sc_prohibit_atoi_atof
local-checks-to-skip += sc_prohibit_strcmp
local-checks-to-skip += sc_useless_cpp_parens

# Stuff maintained by maint.mk:
old_NEWS_hash = d41d8cd98f00b204e9800998ecf8427e
