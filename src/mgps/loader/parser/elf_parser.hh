#pragma once

#include <mgps/loader/plugin_info.hh>

namespace mgps::loader {
	lib_has elf_parser_info(std::string const& path,
	                        size_t ptr_size,
	                        plugin_info& out);
}  // namespace mgps::loader
