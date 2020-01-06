#pragma once
#include <cstdint>
#include <mgps/video/file.hh>
#include <mgps/video/gap.hh>

namespace mgps::video {
	struct playlist {
		ch::milliseconds duration;
		std::vector<file> clips;
		std::vector<gap> jumps;
		travel_ms playback_to_travel(playback_ms) const noexcept;
	};
}  // namespace mgps::video
