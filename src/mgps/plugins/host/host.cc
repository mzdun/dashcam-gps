#include <mgps/plugins/host/host.hh>
#include <string_view>

namespace mgps::plugins::host {
	namespace {
		constexpr auto plugin = std::string_view{"mgps-plugin-"};
	}

	host::~host() = default;

	void host::lookup_plugins(std::error_code& ec) {
		auto so_path = mgps::plugins::host::loader::current_library_path(ec);
		if (ec) return;

		auto dir = so_path.parent_path();
#if !defined(ANDROID) && !defined(__ANDROID__)
		dir /= "mgps-plugins";
#endif
		auto iter = fs::directory_iterator{dir, ec};
		if (ec) return;

		for (auto& entry : iter) {
			std::error_code ec2;
			if (!entry.is_regular_file(ec2)) {
				if (ec2) continue;
				if (!entry.is_symlink(ec2)) continue;
			}

			auto fname = entry.path()
			                 .filename()
#ifdef WIN32
			                 .string()
#endif
			    ;
			auto fname_v = std::string_view(fname.c_str());
			// starts_with?
			if (fname_v.size() > plugin.size() &&
			    fname_v.compare(0, plugin.size(), plugin) == 0) {
				mgps::plugins::host::loader dl{entry
				                                   .path()
#ifdef WIN32
				                                   .string()
#endif
				                                   .c_str()};
				if (!dl) continue;

				auto on_load =
				    dl.resolve<bool(char const*, media_file*)>("mgps_load");
				if (!on_load) {
					(void)mgps::plugins::host::loader::last_error();
					continue;
				}

				auto on_probe = dl.resolve<bool(char const*)>("mgps_probe");
				if (!on_probe) {
					(void)mgps::plugins::host::loader::last_error();
				}

				if (!append(
				        {std::move(fname), std::move(dl), on_probe, on_load})) {
					ec = std::make_error_code(std::errc::operation_canceled);
					return;
				}
			}
		}
	}
}  // namespace mgps::plugins::host
