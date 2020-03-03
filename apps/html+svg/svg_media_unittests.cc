#include <gtest/gtest.h>
#include <sstream>

#include "svg.hh"
#include "svg_unittests.hh"

#include <mgps/library.hh>
#include <mgps/trip.hh>

namespace std {
	inline void PrintTo(string_view const& s, ::std::ostream* os) {
		testing::internal::PrintStringTo({s.data(), s.size()}, os);
	}
}  // namespace std

namespace svg::testing {
	using ::testing::TestWithParam;
	using ::testing::ValuesIn;

	using namespace std::chrono;
	using namespace std::literals;
	using namespace date;
	using namespace date::literals;
	using namespace mgps::track;

	class media_info : public mgps::loader::loader_interface {
		std::vector<mgps::media_file> const* files_;
		auto find(char const* filename) const noexcept {
			return std::find_if(std::begin(*files_), std::end(*files_),
			                    [filename](auto const& file) {
				                    return file.filename == filename;
			                    });
		}

	public:
		explicit media_info(std::vector<mgps::media_file> const* files)
		    : files_{files} {}
		bool probe(char const* filename) const override {
			return find(filename) != std::end(*files_);
		}
		bool load(char const* filename, mgps::media_file* out) const override {
			auto it = find(filename);
			if (it == std::end(*files_)) return false;
			*out = *it;
			return true;
		}
		mgps::loader::plugin_info const& info() const noexcept override {
			static mgps::loader::plugin_info nfo{"fake-plugin", {}, {}};
			return nfo;
		}
	};

	struct media_test {
		std::string expected;
		std::string expected_broken;
		std::vector<mgps::media_file> files{};
	};

	struct FullHtmlSvg : TestWithParam<media_test> {};

	TEST_P(FullHtmlSvg, Report) {
		std::ostringstream os;
		auto const& param = GetParam();

		mgps::library lib{};
		lib.make_plugin<media_info>(&param.files);
		for (auto const& file : param.files)
			lib.add_file(file.filename.c_str());

		auto trips =
		    lib.build(mgps::page::everything, mgps::library::default_gap);

		mgps::svg::html_trace(os, lib, trips);
		auto const actual = os.str();
		ASSERT_EQ(param.expected, actual);
	}

	TEST_P(FullHtmlSvg, Broken) {
		std::ostringstream os;
		auto const& param = GetParam();

		mgps::library lib{};
		lib.make_plugin<media_info>(&param.files);
		for (auto const& file : param.files)
			lib.add_file(file.filename.c_str());

		auto trips =
		    lib.build(mgps::page::everything, mgps::library::default_gap);
		for (auto& trip : trips) {
			ASSERT_NE(trip.owner, nullptr);
			trip.owner = nullptr;
		}

		mgps::svg::html_trace(os, lib, trips);
		auto const actual = os.str();
		ASSERT_EQ(param.expected_broken, actual);
	}

	template <typename Deg>
	static constexpr coordinate coord(Deg deg, NESW direction) {
		return {static_cast<coordinate::rep>(deg * coordinate::precision::den /
		                                     coordinate::precision::num),
		        direction};
	}

	constexpr coordinate operator""_N(unsigned long long deg) {
		return coord(deg, NESW::N);
	}

	constexpr coordinate operator""_E(unsigned long long deg) {
		return coord(deg, NESW::E);
	}

	constexpr coordinate operator""_N(long double deg) {
		return coord(deg, NESW::N);
	}

	constexpr coordinate operator""_E(long double deg) {
		return coord(deg, NESW::E);
	}

	constexpr gps_point pt(coordinate lat,
	                       coordinate lon,
	                       speed_km speed = {},
	                       milliseconds offset = {}) {
		return {{lat, lon}, speed, offset};
	}

	inline std::string output(std::string_view content = {}) {
		return fmt::format(R"(<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
{style}
<body>{content}<div class='versions'>Generated using <span class='module'>mGPS<small> ({version})</small></span>, with help from <span class='module'>fake-plugin</span>.</div>
<iframe name="player-page" src="about:blank"></iframe>
)"sv,
		                   fmt::arg("style", style),
		                   fmt::arg("content", content),
		                   fmt::arg("version", mgps::version::ui));
	}

	using mgps::clip;
	static media_test const media_tests[] = {
	    {output(R"(
<h1>0476-09-04</h1>
<h2>10:55</h2>

<div class="route">
<svg width="3.003003003003003cm" height="3.340840840840841cm" viewBox=" 0 0 2000.0 2225.0" xmlns="http://www.w3.org/2000/svg" style="stroke:#255fb2;stroke-width:60;stroke-linejoin:round;stroke-linecap:round;fill:none">
<rect x="0" y="0" width="2000.0" height="2225.0" rx="600" style="fill:#FEF7E0;stroke:none"/>
<path style="stroke:white;stroke-width:180" d="M1000 1225 L1000 1210 L1000 1195 L1000 1195 L1000 1180 L1000 1165 " />
<circle style="stroke:white;stroke-width:180" cx="1000.0" cy="1225.0" r="40" />
<path style="stroke:white;stroke-width:180" d="M1000 1015 L1000 1000 " />
<circle style="stroke:white;stroke-width:180" cx="1000.0" cy="1015.0" r="40" />
<g>
<path d="M1000 1225 L1000 1210 L1000 1195 L1000 1195 L1000 1180 L1000 1165 " >
<title>0.044 km, 00:05.000 (32 km/h avg)</title>
</path><circle style="stroke-width:60" cx="1000.0" cy="1225.0" r="40" />
</g>
<g>
<path d="M1000 1015 L1000 1000 " >
<title>0.011 km, 00:01.000 (40 km/h avg)</title>
</path><circle style="stroke-width:60" cx="1000.0" cy="1015.0" r="40" />
</g>
</svg>
</div>

<h3>Information</h3>

<table class="info">
<tr><th>Duration:</th><td>00:06</td></tr>
<tr><th>Playlist:</th><td>2 clips</td></tr>
<tr><th>Plot:</th><td>2 lines with 8 points</td></tr>
<tr><th>Distance:</th><td>0.055 km</td></tr>
<tr><th>Average speed:</th><td>33 km/h</td></tr>
</table>

<h3>Playlist</h3>

<table class="clips">
<thead>
<tr><th>Start</th><th>Duration</th><th>Clip</th></tr>
</thead>
<tr><td class="num">00:00</td><td class="num">00:03.000</td><td><a target="player-page" href="file1"><span class="clip-type">&#x1F3AC</span></a></td></tr>
<tr><td class="num">00:03</td><td class="num">00:03.000</td><td><a target="player-page" href="file2"><span class="clip-type" title="Emergency">&#x1F3AC<span class="overlay">&#x1F198;</span></span></a></td></tr>
</td></tr>
</table>
)"sv),
	     output(R"(
<h1>0476-09-04</h1>
<h2>10:55</h2>

<div class="route">
<svg width="3.003003003003003cm" height="3.340840840840841cm" viewBox=" 0 0 2000.0 2225.0" xmlns="http://www.w3.org/2000/svg" style="stroke:#255fb2;stroke-width:60;stroke-linejoin:round;stroke-linecap:round;fill:none">
<rect x="0" y="0" width="2000.0" height="2225.0" rx="600" style="fill:#FEF7E0;stroke:none"/>
<path style="stroke:white;stroke-width:180" d="M1000 1225 L1000 1210 L1000 1195 L1000 1195 L1000 1180 L1000 1165 " />
<circle style="stroke:white;stroke-width:180" cx="1000.0" cy="1225.0" r="40" />
<path style="stroke:white;stroke-width:180" d="M1000 1015 L1000 1000 " />
<circle style="stroke:white;stroke-width:180" cx="1000.0" cy="1015.0" r="40" />
<g>
<path d="M1000 1225 L1000 1210 L1000 1195 L1000 1195 L1000 1180 L1000 1165 " >
<title>0.044 km, 00:05.000 (32 km/h avg)</title>
</path><circle style="stroke-width:60" cx="1000.0" cy="1225.0" r="40" />
</g>
<g>
<path d="M1000 1015 L1000 1000 " >
<title>0.011 km, 00:01.000 (40 km/h avg)</title>
</path><circle style="stroke-width:60" cx="1000.0" cy="1015.0" r="40" />
</g>
</svg>
</div>

<h3>Information</h3>

<table class="info">
<tr><th>Duration:</th><td>00:06</td></tr>
<tr><th>Playlist:</th><td>2 clips</td></tr>
<tr><th>Plot:</th><td>2 lines with 8 points</td></tr>
<tr><th>Distance:</th><td>0.055 km</td></tr>
<tr><th>Average speed:</th><td>33 km/h</td></tr>
</table>

<h3>Playlist</h3>

<table class="clips">
<thead>
<tr><th>Start</th><th>Duration</th><th>Clip</th></tr>
</thead>
<tr><td class="num">00:00</td><td class="num">00:03.000</td><td>&#x2049;&#xFE0F;</td></tr>
<tr><td class="num">00:03</td><td class="num">00:03.000</td><td>&#x2049;&#xFE0F;</td></tr>
</td></tr>
</table>
)"sv),
	     {
	         mgps::media_file{"file1",
	                          clip::normal,
	                          local_days{476_y / sep / 4} + 10h + 55min + 15s,
	                          3s,
	                          {
	                              pt(0_N, 0_E, {40}),
	                              pt(.0001_N, 0_E, {40}, 1s),
	                              pt(.0002_N, 0_E, {40}, 2s),
	                          }},
	         mgps::media_file{"file2",
	                          clip::emergency,
	                          local_days{476_y / sep / 4} + 10h + 55min + 17s,
	                          3s,
	                          {
	                              pt(.0002_N, 0_E, {40}),
	                              pt(.0003_N, 0_E, {40}, 1s),
	                              pt(.0004_N, 0_E, {40}, 2s),
	                              pt(.0014_N, 0_E, {40}, 12s),
	                              pt(.0015_N, 0_E, {40}, 13s),
	                          }},
	     }},
	    {output(R"(
<h1>1947-12-23</h1>
<h2>10:55</h2>

<div class="route">
<svg width="3.003003003003003cm" height="3.340840840840841cm" viewBox=" 0 0 2000.0 2225.0" xmlns="http://www.w3.org/2000/svg" style="stroke:#255fb2;stroke-width:60;stroke-linejoin:round;stroke-linecap:round;fill:none">
<rect x="0" y="0" width="2000.0" height="2225.0" rx="600" style="fill:#FEF7E0;stroke:none"/>
<path style="stroke:white;stroke-width:180" d="M1000 1225 L1000 1210 L1000 1195 L1000 1195 L1000 1195 L1000 1180 L1000 1165 " />
<circle style="stroke:white;stroke-width:180" cx="1000.0" cy="1225.0" r="40" />
<path style="stroke:white;stroke-width:180" d="M1000 1015 L1000 1000 " />
<circle style="stroke:white;stroke-width:180" cx="1000.0" cy="1015.0" r="40" />
<g>
<path d="M1000 1225 L1000 1210 L1000 1195 L1000 1195 L1000 1195 L1000 1180 L1000 1165 " >
<title>0.044 km, 00:06.000 (26 km/h avg)</title>
</path><circle style="stroke-width:60" cx="1000.0" cy="1225.0" r="40" />
</g>
<g>
<path d="M1000 1015 L1000 1000 " >
<title>0.011 km, 00:01.000 (40 km/h avg)</title>
</path><circle style="stroke-width:60" cx="1000.0" cy="1015.0" r="40" />
</g>
</svg>
</div>

<h3>Information</h3>

<table class="info">
<tr><th>Duration:</th><td>00:06</td></tr>
<tr><th>Playlist:</th><td>2 clips</td></tr>
<tr><th>Plot:</th><td>2 lines with 9 points</td></tr>
<tr><th>Distance:</th><td>0.055 km</td></tr>
<tr><th>Average speed:</th><td>28 km/h</td></tr>
</table>

<h3>Playlist</h3>

<table class="clips">
<thead>
<tr><th>Start</th><th>Duration</th><th>Clip</th></tr>
</thead>
<tr><td class="num">00:00.000</td><td class="num">00:03.430</td><td><a target="player-page" href="file1"><span class="clip-type" title="Parking incident">&#x1F3AC<span class="overlay">&#x1F17F;&#xFE0F;</span></span></a></td></tr>
<tr><td class="num">00:03.430</td><td class="num">00:03.000</td><td><a target="player-page" href="file2"><span class="clip-type">&#x1F3AC<span class="overlay">&#x1F615;</span></span></a></td></tr>
</td></tr>
</table>
)"),
	     output(R"(
<h1>1947-12-23</h1>
<h2>10:55</h2>

<div class="route">
<svg width="3.003003003003003cm" height="3.340840840840841cm" viewBox=" 0 0 2000.0 2225.0" xmlns="http://www.w3.org/2000/svg" style="stroke:#255fb2;stroke-width:60;stroke-linejoin:round;stroke-linecap:round;fill:none">
<rect x="0" y="0" width="2000.0" height="2225.0" rx="600" style="fill:#FEF7E0;stroke:none"/>
<path style="stroke:white;stroke-width:180" d="M1000 1225 L1000 1210 L1000 1195 L1000 1195 L1000 1195 L1000 1180 L1000 1165 " />
<circle style="stroke:white;stroke-width:180" cx="1000.0" cy="1225.0" r="40" />
<path style="stroke:white;stroke-width:180" d="M1000 1015 L1000 1000 " />
<circle style="stroke:white;stroke-width:180" cx="1000.0" cy="1015.0" r="40" />
<g>
<path d="M1000 1225 L1000 1210 L1000 1195 L1000 1195 L1000 1195 L1000 1180 L1000 1165 " >
<title>0.044 km, 00:06.000 (26 km/h avg)</title>
</path><circle style="stroke-width:60" cx="1000.0" cy="1225.0" r="40" />
</g>
<g>
<path d="M1000 1015 L1000 1000 " >
<title>0.011 km, 00:01.000 (40 km/h avg)</title>
</path><circle style="stroke-width:60" cx="1000.0" cy="1015.0" r="40" />
</g>
</svg>
</div>

<h3>Information</h3>

<table class="info">
<tr><th>Duration:</th><td>00:06</td></tr>
<tr><th>Playlist:</th><td>2 clips</td></tr>
<tr><th>Plot:</th><td>2 lines with 9 points</td></tr>
<tr><th>Distance:</th><td>0.055 km</td></tr>
<tr><th>Average speed:</th><td>28 km/h</td></tr>
</table>

<h3>Playlist</h3>

<table class="clips">
<thead>
<tr><th>Start</th><th>Duration</th><th>Clip</th></tr>
</thead>
<tr><td class="num">00:00.000</td><td class="num">00:03.430</td><td>&#x2049;&#xFE0F;</td></tr>
<tr><td class="num">00:03.430</td><td class="num">00:03.000</td><td>&#x2049;&#xFE0F;</td></tr>
</td></tr>
</table>
)"),
	     {
	         mgps::media_file{"file1",
	                          clip::parking,
	                          local_days{1947_y / dec / 23} + 10h + 55min + 15s,
	                          3s + 430ms,
	                          {
	                              pt(0_N, 0_E, {40}),
	                              pt(.0001_N, 0_E, {40}, 1s),
	                              pt(.0002_N, 0_E, {40}, 2s),
	                              pt(.0002_N, 0_E, {40}, 3s),
	                          }},
	         mgps::media_file{
	             "file2",
	             clip::unrecognized,
	             local_days{1947_y / dec / 23} + 10h + 55min + 17s + 430ms,
	             3s,
	             {
	                 pt(.0002_N, 0_E, {40}),
	                 pt(.0003_N, 0_E, {40}, 1s),
	                 pt(.0004_N, 0_E, {40}, 2s),
	                 pt(.0014_N, 0_E, {40}, 12s),
	                 pt(.0015_N, 0_E, {40}, 13s),
	             }},
	     }},
	};

	INSTANTIATE_TEST_SUITE_P(Media,
	                         FullHtmlSvg,
	                         ::testing::ValuesIn(media_tests));
}  // namespace svg::testing
