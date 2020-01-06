#include <mgps/track/trace.hh>

namespace mgps::track {
	bool trace::has_points() const noexcept {
		for (auto const& line : lines) {
			if (line.points.empty()) continue;
			return true;
		}
		return false;
	}

	gps_point trace::travel_to_position(travel_ms travel,
	                                    bool interpolate) const noexcept {
		travel -= offset.time_since_epoch();
		auto it_line = std::upper_bound(begin(lines), end(lines),
		                                timeline_item{travel, {}},
		                                [](auto const& lhs, auto const& rhs) {
			                                return lhs.offset < rhs.offset;
		                                });

		if (it_line != begin(lines)) std::advance(it_line, -1);
		if (it_line == end(lines)) return {};

		return it_line->travel_to_position(travel, interpolate);
	}
}  // namespace mgps::track
