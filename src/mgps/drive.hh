#pragma once
#include <cstdint>
#include <mgps/track/trace.hh>
#include <mgps/video/playlist.hh>

namespace mgps {
	struct drive {
		local_milliseconds start;
		video::playlist playlist;
		track::trace trace;

		local_milliseconds travel_to_local(travel_ms travel) const noexcept {
			return start + travel.time_since_epoch();
		}
	};
}  // namespace mgps
