#pragma once

#include <filesystem>
#include <mgps/plugins/host/dynamic_plugin.hh>

namespace mgps::plugins::host {
	namespace fs = std::filesystem;
	struct host {
		virtual ~host();
		virtual bool append(library_info info) = 0;

		void lookup_plugins(std::error_code& ec);
	};
}  // namespace mgps::plugins::host
