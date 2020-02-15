#include <mgps/loader/host.hh>
#include <mgps/plugin/plugin.hh>
#include <mgps/version.hh>
#include <set>
#include <string_view>

namespace mgps::loader {
	namespace {
		using namespace std::literals;

		constexpr auto plugin = std::string_view{"mgps-plugin-"};

		constexpr unsigned char host_build_requirements() {
#ifdef NDEBUG
			return 1u;
#else
			return 0u;
#endif
		}

		enum class lib_has {
			decisions_to_make,
			plugin,
			incompatible_version,
			differing_requirements,
			mangled_info
		};

		enum class info_tag : unsigned char {
			marker = 0xff,
			intro = 0x00,
			version = 0x01,
			name = 0x0a,
			description = 0x0b,
			plugin_version = 0x0c,
			minimal_str = name,
			eof = marker
		};

		info_tag from_uchar(unsigned char uc) {
			return static_cast<info_tag>(uc);
		}

		struct plugin_info_full {
			plugin_info info;
			lib_has result;
		};
		plugin_info_full info_from_plugin_info(plugin::view raw_info) {
			// see plugin::info pattern_length + size of suffix, no strings
			static constexpr size_t minimal_length = 25 + 2;

			if (raw_info.length < minimal_length)
				return {{}, lib_has::mangled_info};

			constexpr auto pattern = "\xFF\x00MGPS-PLUGIN-INFO:\xFF"sv;
			constexpr auto info_start = pattern.size();
			auto prefix = std::string_view{
			    reinterpret_cast<char const*>(raw_info.data), info_start};
			if (prefix != pattern) {
				return {{}, lib_has::mangled_info};
			}

			auto data = raw_info.data + info_start;

			if (from_uchar(*data) == info_tag::version) {
				auto const major = data[1];
				auto const minor = data[2];
				auto const patch = data[3];
				auto const build_type = data[4];

				if (build_type != host_build_requirements()) {
					return {{}, lib_has::differing_requirements};
				}

				if constexpr (mgps::version::major == 0) {
					// pre-1.0 only exact matches
					if (major != 0 || minor != mgps::version::minor ||
					    patch != mgps::version::patch) {
						return {{}, lib_has::incompatible_version};
					}
				} else {
					// post-1.0, different majors are incompatible, for minors
					// if plugin uses newer one, there might be something we
					// do not support yet
					if (major != mgps::version::major ||
					    minor > mgps::version::minor) {
						return {{}, lib_has::incompatible_version};
					}
				}

				if (from_uchar(data[5]) != info_tag::marker) {
					return {{}, lib_has::mangled_info};
				}
				data += 5;
			} else {
				return {{}, lib_has::mangled_info};
			}

			plugin_info info{};
			auto const used_up = static_cast<size_t>(data - raw_info.data);
			auto rest = raw_info.length - used_up;

			while (rest > 1 && from_uchar(*data) == info_tag::marker) {
				--rest;
				++data;
				if (from_uchar(*data) == info_tag::eof) {
					return {std::move(info), lib_has::plugin};
				}

				auto const str_id = from_uchar(*data);
				if (str_id < info_tag::minimal_str) {
					return {std::move(info), lib_has::mangled_info};
				}

				--rest;
				++data;
				if (!rest) {
					return {std::move(info), lib_has::mangled_info};
				}

				auto const length = *data;
				--rest;
				++data;
				if (rest < length) {
					return {std::move(info), lib_has::mangled_info};
				}

				auto str = reinterpret_cast<char const*>(data);
				data += length;
				rest -= length;

				switch (str_id) {
					case info_tag::name:
						info.name = std::string(str, length);
						break;
					case info_tag::description:
						info.description = std::string(str, length);
						break;
					case info_tag::plugin_version:
						info.version = std::string(str, length);
						break;
					default:
						break;
				}
			}

			return {{}, lib_has::mangled_info};
		}
	}  // namespace

	host::~host() = default;

	void host::lookup_plugins(std::error_code& ec) {
		std::error_code ec2;

		auto so_path = mgps::loader::loader::current_library_path(ec);
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
			{
				auto fname_v = std::string_view(fname.c_str());
				// starts_with?
				if (fname_v.size() <= plugin.size() ||
				    fname_v.compare(0, plugin.size(), plugin) != 0)
					continue;
			}

			auto loaded_iter = loaded.lower_bound(spath);
			if (loaded_iter != loaded.end() && *loaded_iter == spath) continue;

			plugin_info_full full_info{{std::move(fname)},
			                           lib_has::decisions_to_make};

			mgps::loader::loader dl{entry
			                            .path()
#ifdef WIN32
			                            .string()
#endif
			                            .c_str()};
			if (!dl) continue;

			using namespace mgps::plugin;
			if (full_info.result == lib_has::decisions_to_make) {
				auto on_info = dl.resolve<view()>("mgps_plugin_info");
				if (!on_info) {
					(void)mgps::loader::loader::last_error();
				} else {
					full_info = info_from_plugin_info(on_info());
					if (full_info.result != lib_has::plugin) continue;
				}
			}

			auto on_load =
			    dl.resolve<bool(char const*, file_info*)>("mgps_load");
			if (!on_load) {
				(void)mgps::loader::loader::last_error();
				continue;
			}

			auto on_probe = dl.resolve<bool(char const*)>("mgps_probe");
			if (!on_probe) {
				(void)mgps::loader::loader::last_error();
			}

			loaded.insert(loaded_iter, spath);
			if (!append({std::move(spath), std::move(dl),
			             std::move(full_info.info), on_probe, on_load})) {
				ec = std::make_error_code(std::errc::operation_canceled);
				return;
			}
		}
	}
}  // namespace mgps::loader
