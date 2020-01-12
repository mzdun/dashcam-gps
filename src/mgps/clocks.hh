#pragma once

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

#include <date/date.h>

#include <filesystem>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace mgps {
	namespace ch = std::chrono;
	namespace fs = std::filesystem;

	using local_ms = date::local_time<ch::milliseconds>;

	// epoch: start of playlist; tracks progression of movie clips
	struct playback_clock {};
	template <typename Duration>
	using playback_time = ch::time_point<playback_clock, Duration>;
	using playback_ms = playback_time<ch::milliseconds>;

	struct timeline_still_item {
		playback_ms offset;
	};

	struct timeline_item {
		playback_ms offset;
		ch::milliseconds duration;
	};

}  // namespace mgps
