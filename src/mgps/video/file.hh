#pragma once
#include <cstdint>
#include <mgps/clocks.hh>

namespace mgps::video {
	enum class clip : int { unrecognized, normal, emergency, parking, other };

	struct file : timeline_item {
		clip type;
		fs::path filename;
	};
}  // namespace mgps::video
