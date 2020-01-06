#pragma once

#include <mgps-70mai/70mai.hh>
#include <mgps/track/polyline.hh>
#include <mgps/video/file.hh>
#include <vector>

namespace mgps {
	struct drive;
}
namespace mgps::mai {
	struct file_info {
		fs::path filename;
		video::clip type{video::clip::unrecognized};
		local_milliseconds date_time;
		ch::milliseconds duration;
		std::vector<track::gps_point> points;
	};

	class loader {
	public:
		bool add_file(fs::path const&) noexcept;
		void add_directory(fs::path const&) noexcept;
		std::vector<drive> build(ch::milliseconds max_stride_gap = {});

	private:
		std::vector<file_info> interim_;
	};
}  // namespace mgps::mai
