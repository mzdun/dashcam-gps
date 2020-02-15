#pragma once

#include <filesystem>
#include <mgps/loader/dynamic_plugin.hh>

namespace mgps::loader {
	namespace fs = std::filesystem;
	struct MGPS_EXPORT host {
		virtual ~host();
		virtual bool append(library_info info) = 0;

		void lookup_plugins(std::error_code& ec);
	};
}  // namespace mgps::loader
