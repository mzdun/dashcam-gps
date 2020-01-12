#pragma once
#include <cstdint>
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
		media_file const* footage(video::media_clip const&) const noexcept;

		local_ms travel_to_local(playback_ms travel) const noexcept {
			return start + travel.time_since_epoch();
		}
	};
}  // namespace mgps
