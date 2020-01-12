#pragma once
#include <cstdint>
#include <mgps/video/media_clip.hh>
#include <vector>

namespace mgps::video {
	struct playlist {
		ch::milliseconds duration;
		std::vector<media_clip> media;
	};
}  // namespace mgps::video
