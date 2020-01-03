#include <mgps/library.hh>

#include <algorithm>
#include <cmath>

namespace mgps::library::track {
	uint64_t distance(point const& p1, point const& p2) {
		auto const lat1_deg = p1.lat.as_float();
		auto const lon1_deg = p1.lon.as_float();
		auto const lat2_deg = p2.lat.as_float();
		auto const lon2_deg = p2.lon.as_float();

		auto deg2rad = [](double angle) {
			return angle * 3.141592653589793 / 180.0;
		};

		auto const dlat = deg2rad(lat2_deg - lat1_deg);
		auto const dlon = deg2rad(lon2_deg - lon1_deg);

		auto const lat1 = deg2rad(lat1_deg);
		auto const lat2 = deg2rad(lat2_deg);

		auto const sin_lat_2 = std::sin(dlat / 2);
		auto const sin_lon_2 = std::sin(dlon / 2);
		auto const a = sin_lat_2 * sin_lat_2 +
		               sin_lon_2 * sin_lon_2 * std::cos(lat1) * std::cos(lat2);
		auto const c = 2.0 * std::asin(std::sqrt(a));

		constexpr uint64_t R_in_metres = 6371008;
		return static_cast<uint64_t>(c * R_in_metres + .5);
	}

	bool plot::has_points() const noexcept {
		for (auto const& seg : segments) {
			if (seg.points.empty()) continue;
			return true;
		}
		return false;
	}
}  // namespace mgps::library::track
