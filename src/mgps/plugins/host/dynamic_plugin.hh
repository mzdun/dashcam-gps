#pragma once

#include <mgps/plugins/host/loader.hh>
#include <mgps/plugins/plugin_interface.hh>

namespace mgps::plugins::host {
	struct library_info {
		std::string name;
		loader library;
		bool (*on_probe)(char const* filename) = nullptr;
		bool (*on_load)(char const* filename, media_file* out) = nullptr;
	};

	class dynamic_plugin : public plugins::loader_interface,
	                       private library_info {
	public:
		dynamic_plugin(library_info info) : library_info{std::move(info)} {}
		bool probe(char const* filename) const override;
		bool load(char const* filename, media_file* out) const override;
	};
}  // namespace mgps::host
