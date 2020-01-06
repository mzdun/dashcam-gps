#pragma once
#include <mgps/track/point.hh>

namespace mgps::track {
	template <typename Point>
	struct boundary_type {
		Point topLeft{};
		Point bottomRight{};

		Point center() const noexcept {
			Point out{};
			out.template get<0>() =
			    (topLeft.template get<0>() + bottomRight.template get<0>()) / 2;
			out.template get<1>() =
			    (topLeft.template get<1>() + bottomRight.template get<1>()) / 2;
			return out;
		}

		auto width() const noexcept {
			return bottomRight.template get<0>() - topLeft.template get<0>();
		}

		auto height() const noexcept {
			return topLeft.template get<1>() - bottomRight.template get<1>();
		}
	};
	using boundary = boundary_type<point>;

}  // namespace mgps::track
