#pragma once
#include <mgps/track/polyline.hh>
#include <vector>

namespace mgps {
	enum class clip : int { unrecognized, normal, emergency, parking, other };

	struct media_file {
		fs::path filename;
		clip type{clip::unrecognized};
		local_ms date_time;
		ch::milliseconds duration;
		std::vector<track::gps_point> points;
	};
}  // namespace mgps
