#pragma once
#include <memory>
#include <mgps/media_file.hh>
#include <mgps/export.hh>

namespace mgps::plugins {
	struct MGPS_EXPORT loader_interface {
		virtual ~loader_interface();
		virtual bool probe(char const* filename) const = 0;
		virtual bool load(char const* filename, media_file* out) const = 0;
	};
	using ptr = std::unique_ptr<loader_interface>;
}  // namespace mgps::plugins
