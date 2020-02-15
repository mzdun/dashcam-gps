#include <mgps/api.hh>
#include <mgps/version.hh>

namespace mgps {
	runtime_version MGPS_EXPORT get_version() noexcept {
		return {version::major,
		        version::minor,
		        version::patch,
		        {version::string, version::string_short, version::stability,
		         version::build_meta, version::ui}};
	}
}  // namespace mgps