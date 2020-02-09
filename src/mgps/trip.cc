#include <mgps/library.hh>
#include <mgps/trip.hh>

namespace mgps {
	media_file const* footage(trip const* that, video::media_clip const& ref) noexcept {
		if (!that->owner) return nullptr;
		return that->owner->footage(ref);
	}
}  // namespace mgps
