#pragma once
#include <memory>
#include <mgps/media_file.hh>

namespace mgps::plugins {
	struct loader_interface {
		virtual ~loader_interface();
		virtual bool probe(fs::path const& filename) const = 0;
		virtual bool load(fs::path const& filename, media_file* out) const = 0;
	};
	using ptr = std::unique_ptr<loader_interface>;
}  // namespace mgps::plugins