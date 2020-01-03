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
				return direction > NESW::E;
			}

			friend constexpr coord operator-(coord const& lhs,
			                                 coord const& rhs) noexcept {
				// (-L) - (-R) = -L + R = - (L-R)
				// (+L) - (+R) =  L - R = + (L-R)
				// (-L) - (+R) = -L - R = - (L+R)
				// (+L) - (-R) =  L + R = + (L+R)
				//
				// Each time, the sign is the same as L's, but if the sings are
				// equal to each other and R > L the subtraction will switch
				// that sign.

				if (lhs.is_neg() == rhs.is_neg()) {
					if (rhs.degs > lhs.degs) {
						using Int = std::underlying_type_t<NESW>;
						auto const int_val = static_cast<Int>(lhs.direction);

						return {rhs.degs - lhs.degs,
						        static_cast<NESW>(int_val ^ 2)};
					}
					return {lhs.degs - rhs.degs, lhs.direction};
				}
				return {lhs.degs + rhs.degs, lhs.direction};
			}

			friend constexpr coord operator+(coord const& lhs,
			                                 coord const& rhs) noexcept {
				// (-L) + (-R) = -L - R = - (L+R)
				// (+L) + (+R) =  L + R = + (L+R)
				// (-L) + (+R) = -L + R = - (L-R)
				// (+L) + (-R) =  L - R = + (L-R)
				//
				// Each time, the sign is the same as L's, but if the sings are
				// not equal to each other and R > L the subtraction will switch
				// that sign.
				//
				// Other than this little change (== to != on is_neg()) the sum
				// _looks_ the same as subtraction.

				if (lhs.is_neg() != rhs.is_neg()) {
					if (rhs.degs > lhs.degs) {
						using Int = std::underlying_type_t<NESW>;
						auto const int_val = static_cast<Int>(lhs.direction);

						return {rhs.degs - lhs.degs,
						        static_cast<NESW>(int_val ^ 2)};
					}
					return {lhs.degs - rhs.degs, lhs.direction};
				}
				return {lhs.degs + rhs.degs, lhs.direction};
			}

			friend constexpr coord operator/(coord const& lhs,
			                                 coord::rep rhs) noexcept {
				return {lhs.degs / rhs, lhs.direction};
			}

			friend constexpr coord operator*(coord const& lhs,
			                                 coord::rep rhs) noexcept {
				return {lhs.degs * rhs, lhs.direction};
			}

			friend constexpr bool operator==(coord const& lhs,
			                                 coord const& rhs) noexcept {
				return lhs.degs == rhs.degs && lhs.direction == rhs.direction;
			}

			friend constexpr bool operator<(coord const& lhs,
			                                coord const& rhs) noexcept {
				auto const lhs_neg = lhs.is_neg();
				auto const rhs_neg = rhs.is_neg();
				if (lhs_neg != rhs_neg) {
					if (!lhs.degs && !rhs.degs)
						return false;  // 0N == 0S, 0W == 0E
					return lhs_neg;
				}
				if (lhs_neg) return lhs.degs > rhs.degs;
				return lhs.degs < rhs.degs;
			}

			friend constexpr bool operator>(coord const& lhs,
			                                coord const& rhs) noexcept {
				return rhs < lhs;
			}

		private:
			constexpr uint64_t fraction_as_minutes_and_thousandths()
				const noexcept {
				using thousandth_of_minute = std::ratio<1, 60 * 1000>::type;
				using scale =
					std::ratio_divide<precision, thousandth_of_minute>::type;

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

			template <size_t id>
			auto& get() noexcept {
				if constexpr (id == 0)
					return lon;
				else if constexpr (id == 1)
					return lat;
			}

			template <size_t id>
			auto const& get() const noexcept {
				if constexpr (id == 0)
					return lon;
				else if constexpr (id == 1)
					return lat;
			}
		};

		template <typename Point>
		struct boundary_type {
			Point topLeft{};
			Point bottomRight{};

			Point center() const noexcept {
				Point out{};
				out.template get<0>() = (topLeft.template get<0>() + bottomRight.template get<0>()) / 2;
				out.template get<1>() = (topLeft.template get<1>() + bottomRight.template get<1>()) / 2;
				return out;
			}

			auto width() const noexcept {
				return bottomRight.template get<0>() -
				       topLeft.template get<0>();
			}

			auto height() const noexcept {
				return bottomRight.template get<1>() -
				       topLeft.template get<1>();
			}
		};
		using boundary = boundary_type<point>;

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
			bool has_points() const noexcept;
			boundary boundary_box() const noexcept {
				// Find the boundary with identity function
				return boundary_box_impl<point>(
				    [](point const& pt) -> point const& { return pt; });
			}

			template <typename Point, typename Filter>
			boundary_type<Point> boundary_box_impl(Filter pred) const noexcept {
				boundary_type<point> b{};

				for (auto const& seg : segments) {
					if (seg.points.empty()) continue;
					auto const& pt = seg.points.front();
					b.topLeft = b.bottomRight = pt;
					break;
				}

				for (auto const& seg : segments) {
					for (auto const& pt : seg.points) {
						if (b.topLeft.template get<0>() > pt.template get<0>())
							b.topLeft.template get<0>() = pt.template get<0>();
						if (b.topLeft.template get<1>() < pt.template get<1>())
							b.topLeft.template get<1>() = pt.template get<1>();
						if (b.bottomRight.template get<0>() <
						    pt.template get<0>())
							b.bottomRight.template get<0>() =
							    pt.template get<0>();
						if (b.bottomRight.template get<1>() >
						    pt.template get<1>())
							b.bottomRight.template get<1>() =
							    pt.template get<1>();
					}
				}

				return b;
			}
		};
	}  // namespace track

	namespace video {
		enum class clip : int {
			unrecognized,
			normal,
			emergency,
			parking,
			other
		};

		struct file : timeline_item {
			clip type;
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
