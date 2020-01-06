#pragma once
#include <cstdint>
#include <mgps/track/boundary.hh>
#include <mgps/track/speed.hh>

namespace mgps::track {
	struct gps_point : point {
		speed kmph;
		ch::milliseconds offset;
	};

	struct polyline : timeline_item {
		std::vector<gps_point> points;
		uint64_t distance() const noexcept {
			if (points.empty()) return 0;
			uint64_t result{};
			gps_point const* previous = nullptr;
			for (auto& pt : points) {
				if (previous) result += track::distance(*previous, pt);
				previous = &pt;
			}
			return result;
		}
		gps_point travel_to_position(travel_ms, bool) const noexcept;
	};
}  // namespace mgps::track
