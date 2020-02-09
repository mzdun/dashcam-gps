#include <mgps/plugins/host/host.hh>
#include <set>
#include <string_view>

namespace mgps::plugins::host {
	namespace {
		constexpr auto plugin = std::string_view{"mgps-plugin-"};
	}

	host::~host() = default;

	void host::lookup_plugins(std::error_code& ec) {
		std::error_code ec2;

		auto so_path = mgps::plugins::host::loader::current_library_path(ec);
		if (ec) return;

		auto dir = fs::weakly_canonical(so_path.parent_path(), ec2);
		ec2.clear();
#if !defined(ANDROID) && !defined(__ANDROID__)
		dir /= "mgps";
#endif
		auto iter = fs::directory_iterator{dir, ec};
		if (ec) return;

		std::set<std::string> loaded{};
		for (auto& entry : iter) {
			auto path = entry.path();

			while (!ec2) {
				if (!fs::is_symlink(path, ec2) || ec2) break;
				auto const link_dir = path.parent_path();
				path = fs::read_symlink(path, ec2);
				if (ec2) break;
				if (!path.has_root_path()) {
					path = fs::canonical(link_dir / path, ec2);
				}
			}
			if (ec2) continue;

			if (!fs::is_regular_file(path, ec2) || ec2) continue;

			if (!path.has_root_path()) {
				path = fs::canonical(dir / path, ec2);
				if (ec2) continue;
			}

			auto spath = path.string();
			auto fname = path.filename()
#ifdef WIN32
			                 .string()
#endif
			    ;
			auto fname_v = std::string_view(fname.c_str());
			// starts_with?
			if (fname_v.size() <= plugin.size() ||
			    fname_v.compare(0, plugin.size(), plugin) != 0)
				continue;

			auto loaded_iter = loaded.lower_bound(spath);
			if (loaded_iter != loaded.end() && *loaded_iter == spath) continue;

			mgps::plugins::host::loader dl{entry
			                                   .path()
#ifdef WIN32
			                                   .string()
#endif
			                                   .c_str()};
			if (!dl) continue;

			using namespace mgps::plugin;
			auto on_load =
			    dl.resolve<bool(char const*, file_info*)>("mgps_load");
			if (!on_load) {
				(void)mgps::plugins::host::loader::last_error();
				continue;
			}

			auto on_probe = dl.resolve<bool(char const*)>("mgps_probe");
			if (!on_probe) {
				(void)mgps::plugins::host::loader::last_error();
			}

			loaded.insert(loaded_iter, spath);
			if (!append({std::move(spath), std::move(fname), std::move(dl),
			             on_probe, on_load})) {
				ec = std::make_error_code(std::errc::operation_canceled);
				return;
			}
		}
	}
}  // namespace mgps::plugins::host
