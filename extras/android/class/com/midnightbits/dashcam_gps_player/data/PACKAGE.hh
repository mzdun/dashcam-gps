#pragma once
#include <com/midnightbits/dashcam_gps_player/PACKAGE.hh>
#include "../PACKAGE.hh"

namespace com::midnightbits::dashcam_gps_player::data {
	struct PACKAGE {
		static constexpr auto package_name() noexcept {
			constexpr auto name =
			    dashcam_gps_player::PACKAGE::package_name() + "/data";
			return name;
		}
	};
}
