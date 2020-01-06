#pragma once

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

#include <date/date.h>

#include <filesystem>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace mgps {
	namespace ch = std::chrono;
	namespace fs = std::filesystem;

	using local_milliseconds = date::local_time<ch::milliseconds>;

	// epoch: start of playlist; tracks progression of recorded time, including
	// gaps between clips
	struct travel_clock {};
	template <typename Duration>
	using travel_time = ch::time_point<travel_clock, Duration>;
	using travel_ms = travel_time<ch::milliseconds>;

	// epoch: start of playlist; tracks progression of movie clips
	struct playback_clock {};
	template <typename Duration>
	using playback_time = ch::time_point<playback_clock, Duration>;
	using playback_ms = playback_time<ch::milliseconds>;

	struct timeline_still_item {
		travel_ms offset;
	};

	struct timeline_item {
		travel_ms offset;
		ch::milliseconds duration;
	};

}  // namespace mgps
