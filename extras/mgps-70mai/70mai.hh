#pragma once

#include <date/date.h>

#include <mgps/plugin/isom.hh>
#include <mgps/plugin/plugin.hh>
#include <mgps/track/speed.hh>
#include <string_view>
#include <vector>

namespace mgps::plugin::isom::mai {
	constexpr auto GPS_box = synth_type("GPS ");

	using local_ms = date::local_time<std::chrono::milliseconds>;
	struct clip_filename_info {
		clip type{clip::unrecognized};
		local_ms ts{};
	};

	clip_filename_info get_filename_info(std::string_view filename);

	using speed_in_metres = track::speed<std::uint32_t, std::ratio<1>>;
	struct gps_point : track::point {
		std::chrono::seconds pos{};
		track::speed_m metres_per_hour{};
		bool valid{false};
	};

	bool read_GPS_box(storage&, box_info const&, std::vector<gps_point>&);
	bool read_moov_mhdr_duration(storage&,
	                             box_info const&,
	                             std::chrono::milliseconds&);
}  // namespace mgps::plugin::isom::mai
