#pragma once
#include <mgps/track/coordinate.hh>

namespace mgps::track {
	struct point {
		coordinate lat, lon;

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
}  // namespace mgps::track
