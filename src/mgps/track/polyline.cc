#include <mgps/api.hh>
#include <mgps/track/point.hh>
#include <mgps/track/polyline.hh>

namespace mgps::track {
	template <size_t id>
	coordinate linear(point const& before,
	                  point const& after,
	                  unsigned long long const fraction,
	                  unsigned long long const whole) {
		auto const& from = before.get<id>();
		auto const& to = after.get<id>();
		return from + (to - from) * fraction / whole;
	}

	uint64_t MGPS_EXPORT distance(track::polyline const* that) noexcept {
		if (that->points.empty()) return 0;
		uint64_t result{};
		gps_point const* previous = nullptr;
		for (auto& pt : that->points) {
			if (previous) result += distance(*previous, pt);
			previous = &pt;
		}
		return result;
	}

	track::gps_point MGPS_EXPORT
	playback_to_position(track::polyline const* that,
	                     playback_ms position,
	                     bool interpolate) noexcept {
		position -= that->offset.time_since_epoch();

		auto const time = [=]() {
			gps_point out{};
			out.offset = position.time_since_epoch();
			return out;
		}();

		auto it = std::upper_bound(begin(that->points), end(that->points), time,
		                           [](auto const& lhs, auto const& rhs) {
			                           return lhs.offset < rhs.offset;
		                           });

		if (it != begin(that->points)) std::advance(it, -1);
		if (it == end(that->points)) return {};

		auto const& sure = *it;
		if (!interpolate) return sure;
		if (sure.offset > position.time_since_epoch()) return sure;

		std::advance(it, 1);
		if (it == end(that->points)) return sure;

		auto const& next = *it;

		auto const fraction = static_cast<unsigned long long>(
		    (position.time_since_epoch() - sure.offset).count());
		auto const whole = static_cast<unsigned long long>(
		    (next.offset - sure.offset).count());
		if (!whole) return sure;

		auto const lon = linear<0>(sure, next, fraction, whole);
		auto const lat = linear<1>(sure, next, fraction, whole);
		return {{lat, lon}, sure.kmph, position.time_since_epoch()};
	}
}  // namespace mgps::track
