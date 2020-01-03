#pragma once

#include <mgps-70mai/70mai.hh>
#include <vector>

namespace mgps::library::mai {
	class loader {
	public:
		bool add_file(fs::path const&) noexcept;
		void add_directory(fs::path const&) noexcept;
		std::vector<trip> build(ch::milliseconds max_stride_gap = {});

	private:
		struct file_info {
			fs::path filename;
			video::clip type{video::clip::unrecognized};
			local_milliseconds date_time;
			ch::milliseconds duration;
			std::vector<track::gps_data> points;
		};

		std::vector<file_info> interim_;
	};
}  // namespace mgps::library::mai
