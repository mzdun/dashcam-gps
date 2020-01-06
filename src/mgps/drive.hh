#pragma once
#include <cstdint>
#include <mgps/track/trace.hh>
#include <mgps/video/playlist.hh>

namespace mgps {
	struct drive {
		local_milliseconds start;
		video::playlist playlist;
		track::trace trace;
	};
}  // namespace mgps
