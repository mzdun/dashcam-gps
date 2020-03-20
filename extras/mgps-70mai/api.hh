#include <70mai.hh>

namespace mgps::plugin::mai::api {
	bool probe(char const* filename, isom::fs_data*);
	bool load(char const* filename, file_info* out, isom::fs_data*);
	bool load(char const* filename,
	          clip force_type,
	          std::chrono::milliseconds force_ts,
	          file_info* out,
	          isom::fs_data*);
}  // namespace mgps::plugin::mai::api
