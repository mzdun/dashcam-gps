#include <mgps/library.hh>
#include <mgps/trip.hh>

namespace mgps {
	media_file const* trip::footage(video::media_clip const& ref) const noexcept {
		if (!owner) return nullptr;
		return owner->footage(ref);
	}
}  // namespace mgps
