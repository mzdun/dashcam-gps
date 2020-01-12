#pragma once
#include <mgps/isom.hh>
#include <mgps/track/point.hh>
#include <mgps/track/speed.hh>
#include <mgps/media_file.hh>
#include <vector>

namespace mgps::isom::mai {
	constexpr auto GPS_box = synth_type("GPS ");

	struct clip_filename_info {
		clip type{clip::unrecognized};
		local_ms ts{};
	};

	clip_filename_info get_filename_info(std::string_view filename);

	struct gps_point : track::point {
		std::chrono::seconds pos{};
		track::speed kmph{};
		bool valid{false};
	};

	bool read_GPS_box(storage&, box_info const&, std::vector<gps_point>&);
	bool read_moov_mhdr_duration(storage&,
	                             box_info const&,
	                             std::chrono::milliseconds&);
}  // namespace mgps::isom::mai
