#pragma once
#include <cstdint>
#include <mgps/export.hh>
#include <mgps/track/trace.hh>
#include <mgps/video/playlist.hh>

namespace mgps {
	class library;
	struct media_file;

	struct trip {
		local_ms start;
		video::playlist playlist;
		track::trace trace;
		library const* owner;

		local_ms travel_to_local(playback_ms travel) const noexcept {
			return start + travel.time_since_epoch();
		}
	};

	MGPS_EXPORT media_file const* footage(
	    trip const*,
	    video::media_clip const&) noexcept;
}  // namespace mgps
