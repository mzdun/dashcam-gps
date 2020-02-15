#pragma once
#include <memory>
#include <mgps/export.hh>
#include <mgps/media_file.hh>

namespace mgps::loader {
	struct plugin_info {
		std::string name{};
		std::string description{};
		std::string version{};
	};

	struct MGPS_EXPORT loader_interface {
		virtual ~loader_interface();
		virtual bool probe(char const* filename) const = 0;
		virtual bool load(char const* filename, media_file* out) const = 0;
		virtual plugin_info const& info() const noexcept = 0;
	};
	using ptr = std::unique_ptr<loader_interface>;
}  // namespace mgps::loader
