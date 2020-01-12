#pragma once
#include <cstdint>
#include <mgps/clocks.hh>

namespace mgps::video {
	struct media_clip : timeline_item {
		enum : size_t { INVALID_REF = std::numeric_limits<size_t>::max() };
		size_t reference{INVALID_REF};
	};
}  // namespace mgps::video
