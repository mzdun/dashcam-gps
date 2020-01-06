#pragma once
#include <cstdint>
#include <mgps/clocks.hh>

namespace mgps::video {
	struct gap {
		playback_ms video_chunk_start;
		travel_ms travel_fix;
	};
}  // namespace mgps::video
