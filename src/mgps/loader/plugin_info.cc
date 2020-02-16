#include <mgps/loader/plugin_info.hh>
#include <mgps/loader/plugin_interface.hh>
#include <mgps/version.hh>

namespace mgps::loader {
	namespace {
		constexpr unsigned char host_build_requirements() {
#ifdef NDEBUG
			return 1u;
#else
			return 0u;
#endif
		}

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
	}  // namespace

	lib_has info_from_plugin_info(plugin::view raw_info, plugin_info& out) {
		using namespace std::literals;

		// see plugin::info pattern_length + size of suffix, no strings
		static constexpr size_t minimal_length = 25 + 2;

		if (raw_info.length < minimal_length) return lib_has::mangled_info;

		constexpr auto pattern = "\xFF\x00MGPS-PLUGIN-INFO:\xFF"sv;
		constexpr auto info_start = pattern.size();
		auto prefix = std::string_view{
		    reinterpret_cast<char const*>(raw_info.data), info_start};
		if (prefix != pattern) {
			return lib_has::mangled_info;
		}

		auto data = raw_info.data + info_start;

		if (from_uchar(*data) == info_tag::version) {
			auto const major = data[1];
			auto const minor = data[2];
			auto const patch = data[3];
			auto const build_type = data[4];

			if (build_type != host_build_requirements()) {
				return lib_has::differing_requirements;
			}

			if constexpr (mgps::version::major == 0) {
				// pre-1.0 only exact matches
				if (major != 0 || minor != mgps::version::minor ||
				    patch != mgps::version::patch) {
					return lib_has::incompatible_version;
				}
			} else {
				// post-1.0, different majors are incompatible, for minors
				// if plugin uses newer one, there might be something we
				// do not support yet
				if (major != mgps::version::major ||
				    minor > mgps::version::minor) {
					return lib_has::incompatible_version;
				}
			}

			if (from_uchar(data[5]) != info_tag::marker) {
				return lib_has::mangled_info;
			}
			data += 5;
		} else {
			return lib_has::mangled_info;
		}

		out = plugin_info{};
		auto const used_up = static_cast<size_t>(data - raw_info.data);
		auto rest = raw_info.length - used_up;

		while (rest > 1 && from_uchar(*data) == info_tag::marker) {
			--rest;
			++data;
			if (from_uchar(*data) == info_tag::eof) {
				return lib_has::plugin;
			}

			auto const str_id = from_uchar(*data);
			if (str_id < info_tag::minimal_str) {
				return lib_has::mangled_info;
			}

			--rest;
			++data;
			if (!rest) {
				return lib_has::mangled_info;
			}

			auto const length = *data;
			--rest;
			++data;
			if (rest < length) {
				return lib_has::mangled_info;
			}

			auto str = reinterpret_cast<char const*>(data);
			data += length;
			rest -= length;

			switch (str_id) {
				case info_tag::name:
					out.name = std::string(str, length);
					break;
				case info_tag::description:
					out.description = std::string(str, length);
					break;
				case info_tag::plugin_version:
					out.version = std::string(str, length);
					break;
				default:
					break;
			}
		}

		return lib_has::mangled_info;
	}
}  // namespace mgps::loader
