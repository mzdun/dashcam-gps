#pragma once
#include <mgps/clip.hh>
#include <mgps/track/polyline.hh>
#include <vector>

namespace mgps {
	struct media_file {
		std::string filename;
		clip type{clip::unrecognized};
		local_ms date_time;
		ch::milliseconds duration;
		std::vector<track::gps_point> points;
	};
}  // namespace mgps
