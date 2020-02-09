#include <mgps/api.hh>
#include <mgps/track/polyline.hh>
#include <mgps/track/trace.hh>

namespace mgps::track {
	gps_point playback_to_position(trace const* that,
	                               playback_ms position,
	                               bool interpolate) noexcept {
		position -= that->offset.time_since_epoch();
		auto it_line = std::upper_bound(begin(that->lines), end(that->lines),
		                                timeline_item{position, {}},
		                                [](auto const& lhs, auto const& rhs) {
			                                return lhs.offset < rhs.offset;
		                                });

		if (it_line != begin(that->lines)) std::advance(it_line, -1);
		if (it_line == end(that->lines)) return {};

		return playback_to_position(&*it_line, position, interpolate);
	}
}  // namespace mgps::track
