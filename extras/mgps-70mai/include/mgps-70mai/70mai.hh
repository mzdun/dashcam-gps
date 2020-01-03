#pragma once
#include <mgps/isom.hh>
#include <mgps/library.hh>

#include <vector>

namespace mgps::isom::mai {
	using namespace library;

	constexpr auto GPS_box = synth_type("GPS ");

	struct clip_filename_info {
		library::video::clip type{library::video::clip::unrecognized};
		local_milliseconds ts{};
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
};  // namespace mgps::isom::mai
