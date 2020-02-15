#pragma once

#include <mgps/clocks.hh>
#include <mgps/export.hh>
#include <mgps/version.hh>
#include <string_view>

namespace mgps::track {
	struct point;
	struct polyline;
	struct trace;
	struct gps_point;
}  // namespace mgps::track

namespace mgps::track {
	uint64_t MGPS_EXPORT distance(track::point const& p1,
	                              track::point const& p2);

	uint64_t MGPS_EXPORT distance(track::polyline const*) noexcept;

	track::gps_point MGPS_EXPORT
	playback_to_position(track::polyline const*,
	                     playback_ms,
	                     bool) noexcept;

	track::gps_point MGPS_EXPORT
	playback_to_position(track::trace const*,
	                     playback_ms,
	                     bool = true) noexcept;
}  // namespace mgps::track

namespace mgps {
	struct runtime_version {
		unsigned major;
		unsigned minor;
		unsigned patch;

		struct {
			std::string_view version;
			std::string_view shorter;
			std::string_view stability;
			std::string_view build_meta;
			std::string_view ui;
		} str;
	};
	runtime_version MGPS_EXPORT get_version() noexcept;
}