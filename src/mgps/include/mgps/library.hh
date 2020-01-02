#pragma once

#include <date/date.h>

#include <cstdint>
#include <filesystem>

namespace mgps {
	namespace ch = std::chrono;
	namespace fs = std::filesystem;

	using local_milliseconds = date::local_time<ch::milliseconds>;
}  // namespace mgps

namespace mgps::library {
	struct timeline_item {
		ch::milliseconds offset;
		ch::milliseconds duration;
	};

	namespace track {
		enum class NESW { N, E, S, W };

		inline std::string to_string(NESW dir) {
			switch (dir) {
				case NESW::N:
					return "N";
				case NESW::E:
					return "E";
				case NESW::S:
					return "S";
				case NESW::W:
					return "W";
			}
			using Int = std::underlying_type_t<NESW>;
			return "NESW(" + std::to_string(static_cast<Int>(dir)) + ")";
		}

		struct coord {
			using precision = std::ratio<1, 100000000ull>::type;
			using rep = std::uint64_t;

			rep degs;
			NESW direction;

			constexpr rep degrees() const noexcept {
				return degs * precision::num / precision::den;
			}

			constexpr rep fraction() const noexcept {
				return degs * precision::num % precision::den;
			}

			constexpr double as_float() const noexcept {
				auto const degs = degrees();
				auto const hundred_millionths =
				    double(fraction()) / precision::den;
				if (is_neg()) return -(degs + hundred_millionths);
				return degs + hundred_millionths;
			}

			constexpr rep minutes() const noexcept {
				return fraction_as_minutes_and_thousandths() / 1000;
			}

			constexpr rep thousandths_of_a_minute() const noexcept {
				return fraction_as_minutes_and_thousandths() / 1000;
			}

			bool valid() const noexcept {
				switch (direction) {
					case NESW::N:
					case NESW::S:
						if (degrees() > 90) return false;
						break;
					case NESW::E:
					case NESW::W:
						if (degrees() > 180) return false;
						break;
				}
				return minutes() < 60;
			}

			constexpr bool is_neg() const noexcept {
				return direction == NESW::S || direction == NESW::W;
			}

			friend constexpr bool operator==(coord const& lhs,
			                                 coord const& rhs) noexcept {
				return lhs.degs == rhs.degs && lhs.direction == rhs.direction;
			}

		private:
			constexpr uint64_t fraction_as_minutes_and_thousandths() const
			    noexcept {
				using thousandth_of_minute = std::ratio<1, 60 * 1000>::type;
				using scale = std::ratio_divide<precision, thousandth_of_minute>::type;

				auto const reminder = fraction();
				return reminder * scale::num / scale::den;
			}
		};

		struct speed {
			std::uint32_t km;

			operator unsigned() const noexcept {
				return static_cast<unsigned>(km);
			}

			friend constexpr bool operator==(speed const& lhs,
			                                 speed const& rhs) noexcept {
				return lhs.km == rhs.km;
			}
		};

		struct point {
			coord lat, lon;
		};

		uint64_t distance(point const& p1, point const& p2);

		struct gps_data : point {
			speed kmph;
			ch::milliseconds time;
		};

		struct segment : timeline_item {
			std::vector<gps_data> points;
			uint64_t distance() const noexcept {
				if (points.empty()) return 0;
				uint64_t result{};
				gps_data const* previous = nullptr;
				for (auto& pt : points) {
					if (previous) result += track::distance(*previous, pt);
					previous = &pt;
				}
				return result;
			}
		};

		struct plot : timeline_item {
			std::vector<segment> segments;
		};
	}  // namespace track

	namespace video {
		struct file : timeline_item {
			fs::path filename;
		};

		struct stride : timeline_item {
			std::vector<file> clips;
		};

	}  // namespace video

	struct trip {
		local_milliseconds start;
		ch::milliseconds duration;
		std::vector<video::stride> strides;
		// TODO: missing gap info (playback_duration and playback-to-trip
		// time mapping)
		track::plot plot;
	};
}  // namespace mgps::library
