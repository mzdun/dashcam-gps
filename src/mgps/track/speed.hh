#pragma once
#include <cstdint>
#include <mgps/clocks.hh>

namespace mgps::track {
	struct speed {
		std::uint32_t km;

		operator unsigned() const noexcept { return static_cast<unsigned>(km); }

		friend constexpr bool operator==(speed const& lhs,
		                                 speed const& rhs) noexcept {
			return lhs.km == rhs.km;
		}
	};
}  // namespace mgps::track
