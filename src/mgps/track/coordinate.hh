#pragma once
#include <cstdint>
#include <mgps/clocks.hh>

namespace mgps::track {
	enum class NESW { N, E, S, W };

	inline constexpr char to_char(NESW dir) {
		switch (dir) {
			case NESW::N:
				return 'N';
			case NESW::E:
				return 'E';
			case NESW::S:
				return 'S';
			case NESW::W:
				return 'W';
		}
		return '?';
	}

	struct coordinate {
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
			auto const full_degs = double(degrees());
			auto const hundred_millionths = double(fraction()) / precision::den;
			if (is_neg()) return -(full_degs + hundred_millionths);
			return full_degs + hundred_millionths;
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

		constexpr bool is_neg() const noexcept { return direction > NESW::E; }

		friend constexpr coordinate operator-(coordinate const& lhs,
		                                      coordinate const& rhs) noexcept {
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

		friend constexpr coordinate operator+(coordinate const& lhs,
		                                      coordinate const& rhs) noexcept {
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

		friend constexpr coordinate operator/(coordinate const& lhs,
		                                      coordinate::rep rhs) noexcept {
			return {lhs.degs / rhs, lhs.direction};
		}

		friend constexpr coordinate operator*(coordinate const& lhs,
		                                      coordinate::rep rhs) noexcept {
			return {lhs.degs * rhs, lhs.direction};
		}

		friend constexpr bool operator==(coordinate const& lhs,
		                                 coordinate const& rhs) noexcept {
			return lhs.degs == rhs.degs && lhs.direction == rhs.direction;
		}

		friend constexpr bool operator<(coordinate const& lhs,
		                                coordinate const& rhs) noexcept {
			auto const lhs_neg = lhs.is_neg();
			auto const rhs_neg = rhs.is_neg();
			if (lhs_neg != rhs_neg) {
				if (!lhs.degs && !rhs.degs) return false;  // 0N == 0S, 0W == 0E
				return lhs_neg;
			}
			if (lhs_neg) return lhs.degs > rhs.degs;
			return lhs.degs < rhs.degs;
		}

		friend constexpr bool operator>(coordinate const& lhs,
		                                coordinate const& rhs) noexcept {
			return rhs < lhs;
		}

	private:
		constexpr uint64_t fraction_as_minutes_and_thousandths() const
		    noexcept {
			using thousandth_of_minute = std::ratio<1, 60 * 1000>::type;
			using scale =
			    std::ratio_divide<precision, thousandth_of_minute>::type;

			auto const reminder = fraction();
			return reminder * scale::num / scale::den;
		}
	};
}  // namespace mgps::track
