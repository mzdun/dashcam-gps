#include <mgps/plugin/plugin.hh>
#include <mgps/plugins/host/dynamic_plugin.hh>

namespace mgps::plugins::host {
	using namespace mgps::plugin;

	namespace {
		struct file_info_impl : file_info {
			media_file* out;
		};

		inline media_file* from_base(file_info* base_ptr) noexcept {
			return static_cast<file_info_impl*>(base_ptr)->out;
		}

		void set_filename(file_info* base_ptr, char const* path) {
			from_base(base_ptr)->filename = path;
		}

		void set_clip(file_info* base_ptr, clip type) noexcept {
			from_base(base_ptr)->type = type;
		}

		void set_timestamp(file_info* base_ptr,
		                   std::chrono::milliseconds ts) noexcept {
			from_base(base_ptr)->date_time = local_ms{ts};
		}
		void set_duration(file_info* base_ptr,
		                  std::chrono::milliseconds dur) noexcept {
			from_base(base_ptr)->duration = dur;
		}

		void before_points(file_info* base_ptr, size_t count) {
			from_base(base_ptr)->points.reserve(count);
		}

		void append_point(
		    file_info* base_ptr,
		    track::point const& pt,
		    track::speed_km kmph,
		    std::chrono::milliseconds offset,
		    [[maybe_unused]] std::chrono::milliseconds duration) noexcept {
			auto& points = from_base(base_ptr)->points;
			points.push_back({pt, kmph, offset});
		}

		file_info_impl conv(media_file* out) {
			static constexpr file_info::vptr_t vptr{
			    set_filename, set_clip,      set_timestamp,
			    set_duration, before_points, append_point};
			file_info base{&vptr};
			return {base, out};
		}
	}  // namespace

	bool dynamic_plugin::probe(char const* filename) const {
		if (!on_probe) return true;
		return on_probe(filename);
	}

	bool dynamic_plugin::load(char const* filename, media_file* out) const {
		if (!on_load) return false;
		auto info = conv(out);
		return on_load(filename, &info);
	}
}  // namespace mgps::plugins::host
