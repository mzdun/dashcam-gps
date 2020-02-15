#pragma once

#include <mgps/loader/loader.hh>
#include <mgps/loader/plugin_interface.hh>

namespace mgps::plugin {
	struct file_info;
}

namespace mgps::loader {
	struct library_info {
		std::string path;
		loader library;
		plugin_info info;
		bool (*on_probe)(char const* filename) = nullptr;
		bool (*on_load)(char const* filename,
		                mgps::plugin::file_info* out) = nullptr;
	};

	class dynamic_plugin : public loader_interface, private library_info {
	public:
		dynamic_plugin(library_info info) : library_info{std::move(info)} {}
		bool probe(char const* filename) const override;
		bool load(char const* filename, media_file* out) const override;
		plugin_info const& info() const noexcept override;
	};
}  // namespace mgps::loader
