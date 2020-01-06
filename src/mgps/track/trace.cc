#include <mgps/track/trace.hh>

namespace mgps::track {
	bool trace::has_points() const noexcept {
		for (auto const& line : lines) {
			if (line.points.empty()) continue;
			return true;
		}
		return false;
	}
}  // namespace mgps::track
