#pragma once

#include <chrono>
#include <cstdint>
#include <mgps/clip.hh>
#include <mgps/track/coordinate.hh>
#include <mgps/track/point.hh>
#include <mgps/track/speed.hh>

#ifdef IMPLEMENTING_PLUGIN
#include <mgps/version.hh>
#endif

namespace mgps::plugin {
	struct file_info {
		struct vptr_t {
			void (*set_filename)(file_info*, char const*);
			void (*set_clip)(file_info*, clip) noexcept;
			void (*set_timestamp)(file_info*,
			                      std::chrono::milliseconds) noexcept;
			void (*set_duration)(file_info*,
			                     std::chrono::milliseconds) noexcept;
			void (*before_points)(file_info*, size_t);
			void (*append_point)(file_info*,
			                     track::point const&,
			                     track::speed_km,
			                     std::chrono::milliseconds,
			                     std::chrono::milliseconds) noexcept;
		};
		vptr_t const* vptr;

		void set_filename(char const* path) { vptr->set_filename(this, path); }
		void set_clip(clip type) noexcept { vptr->set_clip(this, type); }
		void set_timestamp(std::chrono::milliseconds ms) noexcept {
			vptr->set_timestamp(this, ms);
		}
		void set_duration(std::chrono::milliseconds ms) noexcept {
			vptr->set_duration(this, ms);
		}
		void before_points(size_t count) { vptr->before_points(this, count); }
		void append_point(track::point const& pt,
		                  track::speed_km kmph,
		                  std::chrono::milliseconds offset,
		                  std::chrono::milliseconds dur) noexcept {
			vptr->append_point(this, pt, kmph, offset, dur);
		}
	};

	struct view {
		unsigned char const* data;
		size_t length;
	};

#ifdef IMPLEMENTING_PLUGIN
	template <size_t name_length, size_t descr_length, size_t version_length>
	struct info {
		constexpr info(char const (&name)[name_length],
		               char const (&descr)[descr_length],
		               char const (&plugin_version)[version_length]) noexcept {
			static_assert(name_length < 256u,
			              "Name cannot exceed 255 characters.");
			static_assert(descr_length < 256u,
			              "Description cannot exceed 255 characters.");
			auto const name_copied = append_string(pattern_length, 0x0a, name);
			auto const descr_copied = append_string(name_copied, 0x0b, descr);
			auto const version_copied =
			    append_string(descr_copied, 0x0c, plugin_version);
			payload[version_copied] = 0xff;
			payload[version_copied + 1] = 0xff;
		}

		static constexpr size_t pattern_length = 25;
		static constexpr size_t string_prefix = 3;
		static constexpr size_t payload_length =
		    pattern_length + string_prefix * 3 + name_length + descr_length +
		    version_length + 2;
		unsigned char payload[payload_length] = {0xff,
		                                         0x00,
		                                         'M',
		                                         'G',
		                                         'P',
		                                         'S',
		                                         '-',
		                                         'P',
		                                         'L',
		                                         'U',
		                                         'G',
		                                         'I',
		                                         'N',
		                                         '-',
		                                         'I',
		                                         'N',
		                                         'F',
		                                         'O',
		                                         ':',
		                                         0xff,
		                                         0x01,
		                                         version::major,
		                                         version::minor,
		                                         version::patch,
		                                         build_type()};

		constexpr view as_view() const noexcept {
			return {payload, payload_length};
		}

	private:
		template <size_t length>
		constexpr size_t append_string(size_t offset,
		                               unsigned char id,
		                               char const (&str)[length]) noexcept {
			payload[offset] = 0xff;
			payload[offset + 1] = id;
			payload[offset + 2] = length - 1;
			for (size_t ndx = 0; ndx < (length - 1); ++ndx)
				payload[offset + 3 + ndx] =
				    static_cast<unsigned char>(str[ndx]);
			return offset + string_prefix + length - 1;
		}

		static constexpr unsigned char build_type() noexcept {
#ifdef NDEBUG
			return 1;
#else
			return 0;
#endif
		}
	};
#endif
}  // namespace mgps::plugin

#ifdef IMPLEMENTING_PLUGIN
#ifdef __GNUC__
#define API_EXPORT extern "C" __attribute__((visibility("default")))
#else
#define API_EXPORT extern "C" __declspec(dllexport)
#endif

API_EXPORT bool mgps_probe(char const* filename);
API_EXPORT bool mgps_load(char const* filename, mgps::plugin::file_info* out);
API_EXPORT mgps::plugin::view mgps_plugin_info();

#if defined(__APPLE__)
// TODO: Implement section parsing on Mac
#define MGP_PLUGIN_INFO_SECTION \
	__attribute__((section("__TEXT,.mgpsnfo"))) __attribute__((used))
#elif defined(_MSC_VER)
// TODO: Implement section parsing for MSVC
#pragma section(".mgpsnfo", read, shared)
#define MGP_PLUGIN_INFO_SECTION __declspec(allocate(".mgpsnfo"))
#elif defined(__GNUC__) || defined(__clang__)
#define MGP_PLUGIN_INFO_SECTION \
	__attribute__((section(".mgpsnfo"))) __attribute__((used))
#else
#define QT_PLUGIN_METADATA_SECTION
#endif

#define MGPS_PLUGIN_INFO_STICH2(A, B) A##B
#define MGPS_PLUGIN_INFO_STICH(A, B) MGPS_PLUGIN_INFO_STICH2(A, B)

#define MGPS_PLUGIN_INFO(NAME, DESCRIPTION, VERSION)                       \
	constexpr MGP_PLUGIN_INFO_SECTION mgps::plugin::info                   \
	    MGPS_PLUGIN_INFO_STICH(nfo, __LINE__){NAME, DESCRIPTION, VERSION}; \
	API_EXPORT mgps::plugin::view mgps_plugin_info() {                     \
		return MGPS_PLUGIN_INFO_STICH(nfo, __LINE__).as_view();            \
	}

#endif
