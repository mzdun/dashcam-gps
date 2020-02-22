#include <70mai.hh>

namespace mgps::plugin::mai::api {
	bool probe(char const* filename);
	bool load(char const* filename, file_info* out);
	bool load(char const* filename,
	          clip force_type,
	          std::chrono::milliseconds force_ts,
	          file_info* out);
}  // namespace mgps::plugin::mai::api
