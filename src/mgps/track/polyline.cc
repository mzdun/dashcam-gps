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

	gps_point polyline::playback_to_position(playback_ms position,
	                                         bool interpolate) const noexcept {
		position -= offset.time_since_epoch();

		auto const time = [=]() {
			gps_point out{};
			out.offset = position.time_since_epoch();
			return out;
		}();

		auto it = std::upper_bound(begin(points), end(points), time,
		                           [](auto const& lhs, auto const& rhs) {
			                           return lhs.offset < rhs.offset;
		                           });

		if (it != begin(points)) std::advance(it, -1);
		if (it == end(points)) return {};

		auto const& sure = *it;
		if (!interpolate) return sure;
		if (sure.offset > position.time_since_epoch()) return sure;

		std::advance(it, 1);
		if (it == end(points)) return sure;

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
