#include <api.hh>

API_EXPORT bool mgps_probe(char const* filename) {
	return mgps::plugin::mai::api::probe(filename);
}

API_EXPORT bool mgps_load(char const* filename, mgps::plugin::file_info* out) {
	return mgps::plugin::mai::api::load(filename, out);
}

// developed alongside main project, hence the same version...
MGPS_PLUGIN_INFO("70mai",
                 "Reads GPS data from mp4 footage imported from 70mai camera",
                 mgps::version::ui)
