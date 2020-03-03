#pragma once
#include <cstdint>
#include <mgps/track/boundary.hh>
#include <mgps/track/speed.hh>
#include <vector>

namespace mgps::track {
	struct gps_point : point {
		speed_km kmph{};
		ch::milliseconds offset{};
	};

	struct polyline : timeline_item {
		std::vector<gps_point> points;
	};
}  // namespace mgps::track
