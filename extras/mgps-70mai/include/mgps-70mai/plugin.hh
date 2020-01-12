#pragma once

#include <mgps-70mai/70mai.hh>
#include <mgps/plugins/plugin_interface.hh>
#include <vector>

namespace mgps {
	struct trip;
}
namespace mgps::mai {
	struct plugin : plugins::loader_interface {
		bool probe(fs::path const& filename) const override;
		bool load(fs::path const& filename, media_file* out) const override;
	};
}  // namespace mgps::mai
