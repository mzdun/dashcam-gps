#include "70mai_api_unittests.hh"

#include <gtest/gtest.h>

#include <api.hh>
#include <fuzzer/data/xxd_fs_data_unittests.hh>
#include <mgps/loader/dynamic_plugin.hh>
#include <mgps/loader/plugin_info.hh>
#include <mgps/loader/plugin_interface.hh>
#include <mgps/media_file.hh>
#include <mgps/version.hh>
#include <optional>

extern "C" bool mgps_probe(char const* filename);
extern "C" bool mgps_load(char const* filename, mgps::plugin::file_info* out);
extern "C" mgps::plugin::view mgps_plugin_info();

namespace std::chrono {
	inline void PrintTo(milliseconds ms, std::ostream* os) {
		*os << ms.count() << "ms";
	}
}  // namespace std::chrono

namespace mgps::track {
	inline void PrintTo(coordinate coord, std::ostream* os) {
		*os << coord.degs << ' ' << to_char(coord.direction);
	}

	template <typename Rep, typename Ratio>
	inline void PrintTo(speed<Rep, Ratio> sp, std::ostream* os) {
		if constexpr (Ratio::num == 1000)
			*os << sp.speed_over_hour << " km/h";
		else if constexpr (Ratio::num == 1)
			*os << sp.speed_over_hour << " m/h";
		else
			*os << sp.speed_over_hour << " * " << Ratio::num << " m/h";
	}
}  // namespace mgps::track

namespace mgps::plugin::mai::api::testing {
	struct simple_file {
		clip type{clip::unrecognized};
		local_ms date_time;
		ch::milliseconds duration;
		std::initializer_list<track::gps_point> points;
	};

	struct api_test {
		std::string_view filename;
		std::optional<simple_file> loaded;
		bool probed = true;
	};

	void PrintTo(api_test const& test, std::ostream* os) {
		*os << test.filename;
	}

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

		struct actpts {
			std::vector<track::gps_point> const& pts;
			friend std::ostream& operator<<(std::ostream& os,
			                                actpts const& act) {
				for (auto const& pt : act.pts) {
					os << "\npt(" << pt.lat.degs << ", "
					   << to_char(pt.lat.direction) << ", " << pt.lon.degs
					   << ", " << to_char(pt.lon.direction) << ", "
					   << pt.kmph.speed_over_hour << ", ";
					if (pt.offset.count() % 1000)
						os << pt.offset.count() << "ms";
					else
						os << (pt.offset.count() / 1000) << "s";
					os << "),";
				}
				return os;
			}
		};
	}  // namespace

	struct MaiApi : ::testing::TestWithParam<api_test> {
		static loader::dynamic_plugin self;
	};

	loader::library_info syntesize() {
		auto const view = mgps_plugin_info();
		loader::plugin_info info{};
		loader::info_from_plugin_info(view, info);
		return {{}, loader::loader{""}, std::move(info), mgps_probe, mgps_load};
	}
	loader::dynamic_plugin MaiApi::self{syntesize()};

	TEST(MaiApi, PluginInfo) {
		auto const view = mgps_plugin_info();
		loader::plugin_info plug{};
		auto const result = loader::info_from_plugin_info(view, plug);
		auto const& info = MaiApi::self.info();
		ASSERT_EQ(loader::lib_has::plugin, result);
		ASSERT_EQ("70mai", plug.name);
		ASSERT_EQ("Reads GPS data from mp4 footage imported from 70mai camera",
		          plug.description);
		ASSERT_EQ(version::ui, plug.version);

		ASSERT_EQ(plug.name, info.name);
		ASSERT_EQ(plug.description, info.description);
		ASSERT_EQ(plug.version, info.version);
	}

	TEST_P(MaiApi, PluginProbe) {
		auto const& param = GetParam();
		auto const actual = MaiApi::self.probe(param.filename.data());
		auto const expected = false;
		ASSERT_EQ(expected, actual);
	}

	TEST_P(MaiApi, PluginLoad) {
		auto const& param = GetParam();
		auto const actual = MaiApi::self.load(param.filename.data(), nullptr);
		auto const expected = false;
		ASSERT_EQ(expected, actual);
	}

	TEST_P(MaiApi, DynamicProbe) {
		auto const& param = GetParam();
		std::string path;
		path.reserve(std::size(data_dir) + param.filename.size());
		path.append(data_dir);
		path.append(param.filename);
#ifdef _WIN32
		for (auto& c : path) {
			if (c == '/') c = '\\';
		}
#endif
		auto const actual = MaiApi::self.probe(path.c_str());
		auto const expected = param.probed;
		ASSERT_EQ(expected, actual) << "Path is: " << path;
	}

	TEST_P(MaiApi, DynamicLoad) {
		auto const& param = GetParam();
		std::string path;
		path.reserve(std::size(data_dir) + param.filename.size());
		path.append(data_dir);
		path.append(param.filename);
#ifdef _WIN32
		for (auto& c : path) {
			if (c == '/') c = '\\';
		}
#endif

		media_file out{};
		auto const actual = MaiApi::self.load(path.c_str(), &out);
		auto const expected = !!param.loaded;
		ASSERT_EQ(expected, actual);
		if (!actual) return;
		ASSERT_EQ(path, out.filename);
		ASSERT_EQ(param.loaded->type, out.type);
		ASSERT_EQ(param.loaded->date_time, out.date_time);
		ASSERT_EQ(param.loaded->duration, out.duration);

		ASSERT_EQ(param.loaded->points.size(), out.points.size());

		auto it = std::begin(param.loaded->points);

		for (auto const& actual_point : out.points) {
			auto const& expected_point = *it++;
			ASSERT_EQ(expected_point.offset, actual_point.offset)
			    << actpts{out.points};
			ASSERT_EQ(expected_point.lat, actual_point.lat)
			    << actpts{out.points};
			ASSERT_EQ(expected_point.lon, actual_point.lon)
			    << actpts{out.points};
			ASSERT_EQ(expected_point.kmph, actual_point.kmph)
			    << actpts{out.points};
		}
	}

	TEST_P(MaiApi, Probe) {
		auto const& param = GetParam();
		isom::fuzzer::fs_data fs{};
		auto const actual = probe(param.filename.data(), &fs);
		auto const expected = param.probed;
		ASSERT_EQ(expected, actual);
	}

	TEST_P(MaiApi, Load) {
		auto const& param = GetParam();
		isom::fuzzer::fs_data fs{};
		media_file out{};
		auto api_media = conv(&out);
		auto const actual = load(param.filename.data(), &api_media, &fs);
		auto const expected = !!param.loaded;
		ASSERT_EQ(expected, actual);
		if (!actual) return;
		ASSERT_EQ(param.filename, out.filename);
		ASSERT_EQ(param.loaded->type, out.type);
		ASSERT_EQ(param.loaded->date_time, out.date_time);
		ASSERT_EQ(param.loaded->duration, out.duration);

		ASSERT_EQ(param.loaded->points.size(), out.points.size());

		auto it = std::begin(param.loaded->points);

		for (auto const& actual_point : out.points) {
			auto const& expected_point = *it++;
			ASSERT_EQ(expected_point.offset, actual_point.offset)
			    << actpts{out.points};
			ASSERT_EQ(expected_point.lat, actual_point.lat)
			    << actpts{out.points};
			ASSERT_EQ(expected_point.lon, actual_point.lon)
			    << actpts{out.points};
			ASSERT_EQ(expected_point.kmph, actual_point.kmph)
			    << actpts{out.points};
		}
	}

	using namespace std::literals;
	using namespace date;
	using namespace date::literals;

	[[maybe_unused]] constexpr auto N = mgps::track::NESW::N;
	[[maybe_unused]] constexpr auto E = mgps::track::NESW::E;
	[[maybe_unused]] constexpr auto S = mgps::track::NESW::S;
	[[maybe_unused]] constexpr auto W = mgps::track::NESW::W;

	constexpr mgps::track::gps_point pt(uint64_t lat,
	                                    mgps::track::NESW lat_dir,
	                                    uint64_t lon,
	                                    mgps::track::NESW lon_dir,
	                                    std::uint32_t kmph,
	                                    ch::milliseconds offset) {
		return {{{lat, lat_dir}, {lon, lon_dir}}, {kmph}, offset};
	}

	constexpr std::initializer_list<track::gps_point> sane_points{
	    pt(5219058333, N, 2078948333, E, 105, 0s),
	    pt(5219051666, N, 2078906666, E, 105, 1s),
	    pt(5219046666, N, 2078863333, E, 107, 2s),
	    pt(5219041666, N, 2078820000, E, 108, 3s),
	    pt(5219035000, N, 2078776666, E, 110, 4s),
	    pt(5219030000, N, 2078731666, E, 112, 5s),
	    pt(5219025000, N, 2078686666, E, 114, 6s),
	    pt(5219018333, N, 2078640000, E, 116, 7s),
	    pt(5219013333, N, 2078593333, E, 117, 8s),
	    pt(5219008333, N, 2078546666, E, 116, 9s),
	    pt(5219001666, N, 2078501666, E, 114, 10s),
	    pt(5218996666, N, 2078456666, E, 111, 11s),
	    pt(5218993333, N, 2078413333, E, 108, 12s),
	    pt(5218988333, N, 2078370000, E, 106, 13s),
	    pt(5218985000, N, 2078328333, E, 102, 14s),
	    pt(5218981666, N, 2078288333, E, 98, 15s),
	    pt(5218978333, N, 2078248333, E, 95, 16s),
	    pt(5218976666, N, 2078211666, E, 92, 17s),
	    pt(5218978333, N, 2078175000, E, 89, 18s),
	    pt(5218981666, N, 2078138333, E, 87, 19s),
	    pt(5218983333, N, 2078105000, E, 83, 20s),
	    pt(5218986666, N, 2078071666, E, 79, 21s),
	    pt(5218988333, N, 2078040000, E, 76, 22s),
	    pt(5218988333, N, 2078010000, E, 72, 23s),
	    pt(5218988333, N, 2077981666, E, 68, 24s),
	    pt(5218985000, N, 2077955000, E, 66, 25s),
	    pt(5218980000, N, 2077930000, E, 66, 26s),
	    pt(5218973333, N, 2077905000, E, 65, 27s),
	    pt(5218965000, N, 2077883333, E, 63, 28s),
	    pt(5218956666, N, 2077861666, E, 59, 29s),
	    pt(5218950000, N, 2077843333, E, 50, 30s),
	    pt(5218945000, N, 2077828333, E, 42, 31s),
	    pt(5218938333, N, 2077815000, E, 37, 32s),
	    pt(5218935000, N, 2077803333, E, 32, 33s),
	    pt(5218930000, N, 2077793333, E, 28, 34s),
	    pt(5218926666, N, 2077783333, E, 25, 35s),
	    pt(5218923333, N, 2077775000, E, 23, 36s),
	    pt(5218920000, N, 2077768333, E, 20, 37s),
	    pt(5218918333, N, 2077761666, E, 17, 38s),
	    pt(5218916666, N, 2077756666, E, 15, 39s),
	    pt(5218915000, N, 2077751666, E, 12, 40s),
	    pt(5218913333, N, 2077748333, E, 10, 41s),
	    pt(5218911666, N, 2077745000, E, 8, 42s),
	    pt(5218910000, N, 2077741666, E, 7, 43s),
	    pt(5218910000, N, 2077740000, E, 5, 44s),
	    pt(5218910000, N, 2077738333, E, 3, 45s),
	    pt(5218908333, N, 2077736666, E, 1, 46s),
	    pt(5218908333, N, 2077736666, E, 1, 47s),
	    pt(5218908333, N, 2077736666, E, 1, 48s),
	    pt(5218908333, N, 2077736666, E, 0, 49s),
	    pt(5218906666, N, 2077733333, E, 14, 50s),
	    pt(5218903333, N, 2077725000, E, 22, 51s),
	    pt(5218900000, N, 2077716666, E, 24, 52s),
	    pt(5218896666, N, 2077708333, E, 29, 53s),
	    pt(5218890000, N, 2077700000, E, 31, 54s),
	    pt(5218881666, N, 2077695000, E, 34, 55s),
	    pt(5218873333, N, 2077695000, E, 37, 56s),
	    pt(5218863333, N, 2077701666, E, 41, 57s),
	    pt(5218853333, N, 2077710000, E, 46, 58s),
	    pt(5218843333, N, 2077721666, E, 52, 59s),
	    pt(5218843333, N, 2077721666, E, 52, 60s)};

	constexpr std::initializer_list<track::gps_point> points_12_12{
	    pt(5219058333, N, 2078948333, E, 105, 0ms),
	    pt(5219051666, N, 2078906666, E, 33594, 1s),
	    pt(5219046666, N, 2078863333, E, 107, 2s),
	    pt(5219035000, N, 2078885000, E, 110, 4s)};

	constexpr std::initializer_list<track::gps_point> points_12_13{
	    pt(5219058333, N, 2078948333, E, 105, 0ms)};

	constexpr std::initializer_list<track::gps_point> points_12_14{
	    pt(4247318333, N, 2078906666, E, 105, 1s)};

	constexpr std::initializer_list<track::gps_point> points_12_15{
	    pt(5219058333, N, 2078948333, E, 105, 0ms),
	    pt(5219051666, N, 2078906666, E, 105, 1s),
	    pt(5219046666, N, 2078863333, E, 107, 2s),
	    pt(5219041666, N, 2078820000, E, 16624, 3s),
	    pt(5219030000, N, 2078731666, E, 112, 5s)};

	constexpr std::initializer_list<track::gps_point> points_12_16{
	    pt(5219058333, N, 2078948333, E, 105, 0ms),
	    pt(5219051666, N, 2078906666, E, 105, 1s),
	    pt(5219046666, N, 2078863333, E, 107, 2s),
	    pt(5219041666, N, 2078820000, E, 108, 3s),
	    pt(5219035000, N, 2078776666, E, 110, 4s),
	    pt(5219030000, N, 2078731666, E, 112, 5s)};

	constexpr std::initializer_list<track::gps_point> points_12_17{
	    pt(5219058333, N, 2078948333, E, 105, 0s),
	    pt(5219051666, N, 2078906666, E, 105, 1s),
	    pt(5219046666, N, 2078863333, E, 107, 2s),
	    pt(5219041666, N, 2078820000, E, 108, 3s),
	    pt(5219035000, N, 2078776666, E, 110, 4s),
	    pt(5219030000, N, 2078731666, E, 112, 5s),
	    pt(5219025000, N, 2078686666, E, 114, 6s),
	    pt(5219018333, N, 2078640000, E, 116, 7s),
	    pt(5219013333, N, 2078593333, E, 117, 8s),
	    pt(5219008333, N, 2078546666, E, 116, 9s),
	    pt(5219001666, N, 2078501666, E, 114, 10s),
	    pt(5218996666, N, 2078456666, E, 111, 11s),
	    pt(5218993333, N, 2078413333, E, 108, 12s),
	    pt(5218988333, N, 2078370000, E, 106, 13s),
	    pt(5218985000, N, 2078328333, E, 102, 14s),
	    pt(5218981666, N, 2078288333, E, 98, 15s),
	    pt(5218978333, N, 2078248333, E, 95, 16s),
	    pt(5218976666, N, 2078211666, E, 92, 17s),
	    pt(5218978333, N, 2078175000, E, 89, 18s),
	    pt(5218981666, N, 2078138333, E, 87, 19s),
	    pt(5218983333, N, 2078105000, E, 83, 20s),
	    pt(5218986666, N, 2078071666, E, 79, 21s),
	    pt(5218988333, N, 2078040000, E, 76, 22s),
	    pt(5218988333, N, 2078010000, E, 72, 23s),
	    pt(5218988333, N, 2077981666, E, 68, 24s),
	    pt(5218985000, N, 2077955000, E, 66, 25s),
	    pt(5218980000, N, 2077930000, E, 66, 26s),
	    pt(5218973333, N, 2077905000, E, 65, 27s),
	    pt(5218965000, N, 2077883333, E, 63, 28s),
	    pt(5218956666, N, 2077861666, E, 59, 29s),
	    pt(5218950000, N, 2077843333, E, 50, 30s),
	    pt(5218945000, N, 2077828333, E, 42, 31s),
	    pt(5218938333, N, 2077815000, E, 37, 32s),
	    pt(5218935000, N, 2077803333, E, 32, 33s),
	    pt(5218930000, N, 2077793333, E, 28, 34s),
	    pt(5218926666, N, 2077783333, E, 25, 35s),
	    pt(5218923333, N, 2077775000, E, 23, 36s),
	    pt(5218920000, N, 2077768333, E, 20, 37s),
	    pt(5218918333, N, 2077761666, E, 17, 38s),
	    pt(5218916666, N, 2077756666, E, 15, 39s),
	    pt(5218915000, N, 2077751666, E, 12, 40s),
	    pt(5218913333, N, 2077748333, E, 10, 41s),
	    pt(5218911666, N, 2077745000, E, 8, 42s),
	    pt(5218910000, N, 2077741666, E, 7, 43s),
	    pt(5218910000, N, 2077740000, E, 5, 44s),
	    pt(5218910000, N, 2077738333, E, 3, 45s),
	    pt(5218908333, N, 2077736666, E, 1, 46s),
	    pt(5218908333, N, 2077736666, E, 1, 47s),
	    pt(5218908333, N, 2077736666, E, 1, 48s),
	    pt(5218908333, N, 2077736666, E, 0, 49s),
	    pt(5218906666, N, 2077733333, E, 14, 50s),
	    pt(5218903333, N, 2077725000, E, 22, 51s),
	    pt(5218900000, N, 2077716666, E, 24, 52s),
	    pt(5218896666, N, 2077708333, E, 29, 53s),
	    pt(5218890000, N, 2077700000, E, 31, 54s),
	    pt(5218881666, N, 2077695000, E, 34, 55s)};

	constexpr std::initializer_list<track::gps_point> points_12_19{
	    pt(5219058333, N, 2078948333, E, 105, 0ms)};

	constexpr std::initializer_list<track::gps_point> points_12_26{
	    pt(5219058333, N, 2078948333, E, 105, 0ms),
	    pt(5219051666, N, 2078906666, E, 105, 1s),
	    pt(5219046666, N, 2078863333, E, 107, 2s),
	    pt(5219041666, N, 2078820000, E, 108, 3s),
	    pt(5219035000, N, 2078776666, E, 110, 4s),
	    pt(5219030000, N, 2078731666, E, 112, 5s)};

	constexpr std::initializer_list<track::gps_point> points_12_32{
	    pt(5219058333, N, 2078948333, E, 105, 0ms),
	    pt(5219051666, N, 2078906666, E, 105, 1s),
	    pt(5219046666, N, 2078863333, E, 107, 2s),
	    pt(5219041666, N, 2078820000, E, 108, 3s),
	    pt(5219035000, N, 2078776666, E, 110, 4s),
	    pt(5219030000, N, 2078731666, E, 112, 5s)};

	constexpr api_test tests[] = {
	    {"NO20191130-120156-000121.MP4"sv,
	     simple_file{
	         clip::normal,
	         local_days{2019_y / nov / 30} + 12h + 1min + 56s,
	         1s,
	         sane_points,
	     }},
	    {"NO20191130-120256-000122.MP4"sv,
	     simple_file{
	         clip::normal,
	         local_days{2019_y / nov / 30} + 12h + 2min + 56s,
	         1s,
	         sane_points,
	     }},
	    {"NO20191130-120356-000123.MP4"sv, std::nullopt},
	    {"NO20191130-120456-000124.MP4"sv, std::nullopt},
	    {"NO20191130-120556-000125.MP4"sv,
	     simple_file{
	         clip::normal,
	         local_days{2019_y / nov / 30} + 12h + 5min + 56s,
	         1s,
	         {},
	     }},
	    {"NO20191130-120656-000126.MP4"sv, std::nullopt},
	    {"NO20191130-120756-000127.MP4"sv,
	     simple_file{
	         clip::normal,
	         local_days{2019_y / nov / 30} + 12h + 7min + 56s,
	         1s,
	         sane_points,
	     }},
	    {"XA20191130-120856-000128.MP4"sv,
	     simple_file{
	         clip::other,
	         local_days{2019_y / nov / 30} + 12h + 8min + 56s,
	         255086697644032ms,
	         sane_points,
	     }},
	    {"BY20191130-120956-000129.MP4"sv, std::nullopt},
	    {"EV20191130-121056-000130.MP4"sv, std::nullopt},
	    {"EV20191130-121156-000131.MP4"sv, std::nullopt},
	    {"EV20191130-121256-000132.MP4"sv,
	     simple_file{
	         clip::emergency,
	         local_days{2019_y / nov / 30} + 12h + 12min + 56s,
	         1s,
	         points_12_12,
	     }},
	    {"EV20191130-121356-000133.MP4"sv,
	     simple_file{
	         clip::emergency,
	         local_days{2019_y / nov / 30} + 12h + 13min + 56s,
	         1s,
	         points_12_13,
	     }},
	    {"EV20191130-121456-000134.MP4"sv,
	     simple_file{
	         clip::emergency,
	         local_days{2019_y / nov / 30} + 12h + 14min + 56s,
	         1s,
	         points_12_14,
	     }},
	    {"PA20191130-121556-000135.MP4"sv,
	     simple_file{
	         clip::parking,
	         local_days{2019_y / nov / 30} + 12h + 15min + 56s,
	         1s,
	         points_12_15,
	     }},
	    {"PA20191130-121656-000136.MP4"sv,
	     simple_file{
	         clip::parking,
	         local_days{2019_y / nov / 30} + 12h + 16min + 56s,
	         1s,
	         points_12_16,
	     }},
	    {"PA20191130-121756-000137.MP4"sv,
	     simple_file{
	         clip::parking,
	         local_days{2019_y / nov / 30} + 12h + 17min + 56s,
	         1s,
	         points_12_17,
	     }},
	    {"PA20191130-121856-000138.MP4"sv,
	     simple_file{
	         clip::parking,
	         local_days{2019_y / nov / 30} + 12h + 18min + 56s,
	         1s,
	         sane_points,
	     }},
	    {"NO20191130-121956-000139.MP4"sv,
	     simple_file{
	         clip::normal,
	         local_days{2019_y / nov / 30} + 12h + 19min + 56s,
	         1s,
	         points_12_19,
	     }},
	    {"NO20191130-122056-000140.MP4"sv, std::nullopt},
	    {"NO20191130-122156-000141.MP4"sv, std::nullopt},
	    {"EV20191130-122256-000142.MP4"sv, std::nullopt},
	    {"EV20191130-122356-000143.MP4"sv, std::nullopt},
	    {"EV20191130-122456-000144.MP4"sv, std::nullopt},
	    {"PA20191130-122556-000145.MP4"sv, std::nullopt},
	    {"PA20191130-122656-000146.MP4"sv,
	     simple_file{
	         clip::parking,
	         local_days{2019_y / nov / 30} + 12h + 26min + 56s,
	         1s,
	         points_12_26,
	     }},
	    {"NO20191130-122756-000147.MP4"sv, std::nullopt},
	    {"NO20191130-122856-000148.MP4"sv, std::nullopt},
	    {"NO20191130-122956-000149.MP4"sv,
	     simple_file{
	         clip::normal,
	         local_days{2019_y / nov / 30} + 12h + 29min + 56s,
	         1s,
	         {},
	     }},
	    {"XX20191130-123056-000150.MP4"sv, std::nullopt},
	    {"YY20191130-123156-000151.MP4"sv, std::nullopt},
	    {"ZZ20191130-123256-000152.MP4"sv,
	     simple_file{
	         clip::other,
	         local_days{2019_y / nov / 30} + 12h + 32min + 56s,
	         1s,
	         points_12_32,
	     }},
	    {"AA20191130-123356-000153.MP4"sv, std::nullopt},
	    {"BB20191130-123456-000154.MP4"sv, std::nullopt, false},
	    {"NO20191130-123556-000155.MP4"sv, std::nullopt, false},
	    {"NO20191130-123656-000156.MP4"sv, std::nullopt, false},
	};

	INSTANTIATE_TEST_SUITE_P(Files, MaiApi, ::testing::ValuesIn(tests));
}  // namespace mgps::plugin::mai::api::testing
