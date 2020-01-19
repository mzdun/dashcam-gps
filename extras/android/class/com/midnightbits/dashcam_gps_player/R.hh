#pragma once

#include <com/midnightbits/dashcam_gps_player/PACKAGE.hh>

namespace com::midnightbits::dashcam_gps_player {
	struct R_type {
		struct string_type {
			int page_everything;
			int page_emergency;
			int page_parking;
			void load();
		} string;
		struct drawable_type {
			int ic_folder_everything;
			int ic_folder_emergency;
			int ic_folder_parking;
			void load();
		} drawable;

		void load() {
			string.load();
			drawable.load();
		}
	};
}  // namespace com::midnightbits::dashcam_gps_player
