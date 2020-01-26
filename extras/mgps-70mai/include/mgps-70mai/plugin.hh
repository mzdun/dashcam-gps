#pragma once

#include <mgps-70mai/70mai.hh>
#include <mgps/plugins/plugin_interface.hh>
#include <vector>

namespace mgps {
	struct trip;
}

namespace mgps::mai::api {
	bool probe(char const* filename);
	bool load(char const* filename, media_file* out);
}  // namespace mgps::mai::api

namespace mgps::mai {
	struct plugin : plugins::loader_interface {
		bool probe(char const* filename) const override;
		bool load(char const* filename, media_file* out) const override;
	};
}  // namespace mgps::mai
