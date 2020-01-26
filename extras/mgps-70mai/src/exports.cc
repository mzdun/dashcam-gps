#include <mgps-70mai/plugin.hh>
#include <mgps/plugins/client.hh>

API_EXPORT bool mgps_probe(char const* filename) {
	return mgps::mai::api::probe(filename);
}

API_EXPORT bool mgps_load(char const* filename, mgps::media_file* out) {
	return mgps::mai::api::load(filename, out);
}
