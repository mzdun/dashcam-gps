#pragma once
#include <mgps/track/polyline.hh>

namespace mgps::track {
	struct trace : timeline_item {
		std::vector<polyline> lines;
		bool has_points() const noexcept;
		boundary boundary_box() const noexcept {
			// Find the boundary with identity function
			return boundary_box_impl<point>(
			    [](point const& pt) -> point const& { return pt; });
		}

		template <typename Point, typename Filter>
		boundary_type<Point> boundary_box_impl(Filter pred) const noexcept {
			boundary_type<Point> b{};

			for (auto const& line : lines) {
				if (line.points.empty()) continue;
				b.topLeft = b.bottomRight = pred(line.points.front());
				break;
			}

			for (auto const& line : lines) {
				for (auto const& pt_ : line.points) {
					auto&& pt = pred(pt_);
					if (b.topLeft.template get<0>() > pt.template get<0>())
						b.topLeft.template get<0>() = pt.template get<0>();
					if (b.topLeft.template get<1>() < pt.template get<1>())
						b.topLeft.template get<1>() = pt.template get<1>();
					if (b.bottomRight.template get<0>() < pt.template get<0>())
						b.bottomRight.template get<0>() = pt.template get<0>();
					if (b.bottomRight.template get<1>() > pt.template get<1>())
						b.bottomRight.template get<1>() = pt.template get<1>();
				}
			}

			return b;
		}
	};
}  // namespace mgps::track
