#pragma once

#include <chrono>
#include <cstdint>
#include <mgps/clip.hh>
#include <mgps/track/coordinate.hh>
#include <mgps/track/point.hh>
#include <mgps/track/speed.hh>

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
}  // namespace mgps::plugin

#ifdef IMPLEMENTING_PLUGIN
#ifdef __GNUC__
#define API_EXPORT extern "C" __attribute__((visibility("default")))
#else
#define API_EXPORT extern "C" __declspec(dllexport)
#endif

API_EXPORT bool mgps_probe(char const* filename);
API_EXPORT bool mgps_load(char const* filename, mgps::plugin::file_info* out);
#endif
