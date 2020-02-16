#pragma once

#include <mgps/plugin/plugin.hh>

namespace mgps::loader {
	struct plugin_info;

	enum class lib_has {
		decisions_to_make,
		plugin,
		existance_issues,
		incorrect_binary_format,  // not a PE-COFF/ELF/Mach-O
		unexpected_architecture,
		no_needed_segments,
		incompatible_version,
		differing_requirements,
		mangled_info
	};

	lib_has info_from_plugin_info(plugin::view raw_info, plugin_info& out);
}  // namespace mgps::loader
