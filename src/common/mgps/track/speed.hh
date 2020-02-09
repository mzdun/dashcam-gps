#pragma once
#include <cstdint>
#include <mgps/clocks.hh>

namespace mgps::track {
	template <typename Rep, typename Ratio>
	struct speed {
		using precision = typename Ratio::type;
		using rep = Rep;

		rep speed_over_hour;

		operator unsigned() const noexcept {
			return static_cast<unsigned>(speed_over_hour);
		}

		friend constexpr bool operator==(speed const& lhs,
		                                 speed const& rhs) noexcept {
			return lhs.speed_over_hour == rhs.speed_over_hour;
		}
	};

	template <typename Type>
	struct is_speed : std::false_type {};
	template <typename Type>
	constexpr bool is_speed_v = is_speed<Type>::value;

	template <typename Rep, typename Ratio>
	struct is_speed<speed<Rep, Ratio>> : std::true_type {};

	using speed_km = speed<std::uint32_t, std::kilo>;
	using speed_m = speed<std::uint32_t, std::ratio<1>>;

	template <typename SpeedTo, typename Rep, typename Precision>
	std::enable_if_t<is_speed_v<SpeedTo>, SpeedTo> speed_cast(
	    speed<Rep, Precision> const& from) {
		using CF = std::ratio_divide<Precision, typename SpeedTo::precision>;
		// std::chrono::duration_cast

		using ToRep = typename SpeedTo::rep;
		using CR = std::common_type_t<ToRep, Rep, intmax_t>;

		constexpr bool Num_is_one = CF::num == 1;
		constexpr bool Den_is_one = CF::den == 1;
		if constexpr (Den_is_one) {
			if constexpr (Num_is_one) {
				return {static_cast<ToRep>(from.speed_over_hour)};
			} else {
				return {
				    static_cast<ToRep>(static_cast<CR>(from.speed_over_hour) *
				                       static_cast<CR>(CF::num))};
			}
		} else {
			if constexpr (Num_is_one) {
				return {
				    static_cast<ToRep>(static_cast<CR>(from.speed_over_hour) /
				                       static_cast<CR>(CF::den))};
			} else {
				return {static_cast<ToRep>(
				    static_cast<CR>(from.speed_over_hour) *
				    static_cast<CR>(CF::num) / static_cast<CR>(CF::den))};
			}
		}
	}
}  // namespace mgps::track
