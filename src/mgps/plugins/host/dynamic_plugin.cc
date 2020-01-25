#include <mgps/plugins/host/dynamic_plugin.hh>

namespace mgps::plugins::host {
	bool dynamic_plugin::probe(char const* filename) const {
		if (!on_probe) return true;
		return on_probe(filename);
	}

	bool dynamic_plugin::load(char const* filename, media_file* out) const {
		if (!on_load) return false;
		return on_load(filename, out);
	}
}  // namespace mgps::plugins::host
