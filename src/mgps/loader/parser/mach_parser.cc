#include <mgps/loader/parser/file_ptr.hh>
#include <mgps/loader/parser/mach_parser.hh>
#include <vector>

namespace mgps::loader {
	lib_has mach_parser_info(std::string const& path,
	                         size_t ptr_size,
	                         plugin_info& out) {
		auto file = file_ptr{std::fopen(path.c_str(), "rb")};
		if (!file) return lib_has::existance_issues;

		return lib_has::no_needed_segments;
	}
}  // namespace mgps::loader
