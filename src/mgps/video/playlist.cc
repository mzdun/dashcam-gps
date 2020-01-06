#include <algorithm>
#include <mgps/video/playlist.hh>

namespace mgps::video {
	travel_ms playlist::playback_to_travel(playback_ms millis) const noexcept {
		auto it = std::upper_bound(
		    begin(jumps), end(jumps), video::gap{millis, {}},
		    [](auto const& lhs, auto const& rhs) {
			    return lhs.video_chunk_start < rhs.video_chunk_start;
		    });

		if (it != begin(jumps)) {
			auto const& jump = *std::prev(it);
			return jump.travel_fix + (millis - jump.video_chunk_start);
		}

		return travel_ms{millis.time_since_epoch()};
	}
}  // namespace mgps::video
