#include <api.hh>
#include <iostream>

namespace mgps {
	using namespace plugin;

	void set_filename(file_info*, char const* path) {
		std::cout << "filename: " << path << '\n';
	}

	void set_clip(file_info*, clip) noexcept {}
	void set_timestamp(file_info*, std::chrono::milliseconds) noexcept {}

	void set_duration(file_info*, std::chrono::milliseconds dur) noexcept {
		std::cout << "duration: " << dur.count() << "ms\n";
	}

	void before_points(file_info*, size_t count) {
		std::cout << "allocate: " << count << " point(s)\n";
	}

	void append_point(file_info*,
	                  track::point const& pt,
	                  track::speed_km kmph,
	                  std::chrono::milliseconds offset,
	                  std::chrono::milliseconds duration) noexcept {
		std::cout << ":   " << pt.lat.as_float() << ' '
		          << to_char(pt.lat.direction) << ' ' << pt.lon.as_float()
		          << ' ' << to_char(pt.lon.direction) << ", "
		          << kmph.speed_over_hour << " km/h, " << offset.count()
		          << " ms, " << duration.count() << " ms\n";
	}

	file_info create() {
		static constexpr file_info::vptr_t vptr{set_filename,  set_clip,
		                                        set_timestamp, set_duration,
		                                        before_points, append_point};
		return {&vptr};
	}
}  // namespace mgps

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "fuzzer-70mai-boxes <clip>\n";
		return 1;
	}

	using namespace mgps::plugin;

	file_info out = mgps::create();
	auto const result = mai::api::load(argv[1], mgps::clip::normal, {}, &out);
	if (!result) {
		std::cerr << "fuzzer-70mai-boxes: cannot open " << argv[1] << "\n";
		return 1;
	}
}
