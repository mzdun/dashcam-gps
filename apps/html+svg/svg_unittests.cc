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

	struct plug_info : mgps::loader::loader_interface {
		plug_info(mgps::loader::plugin_info const& nfo) : nfo_{nfo} {}
		bool probe(char const* filename) const override { return false; }
		bool load(char const* filename, mgps::media_file* out) const override {
			return false;
		}
		mgps::loader::plugin_info const& info() const noexcept override {
			return nfo_;
		}
		mgps::loader::plugin_info nfo_;
	};

	struct svg_test {
		std::string expected;
		std::vector<mgps::trip> trips{};
		std::vector<mgps::loader::plugin_info> plugins{};
	};

	struct HtmlSvg : TestWithParam<svg_test> {};

	TEST_P(HtmlSvg, Report) {
		std::ostringstream os;
		auto const& param = GetParam();

		mgps::library lib{};
		for (auto const& info : param.plugins)
			lib.make_plugin<plug_info>(info);

		mgps::svg::html_trace(os, lib, param.trips);
		auto const actual = os.str();
		ASSERT_EQ(param.expected, actual);
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

	inline std::string with_plugins(std::string_view plugins = {}) {
		return fmt::format(R"(<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
{style}
<body><div class='versions'>Generated using <span class='module'>mGPS<small> ({version})</small></span>{plugins}</div>
<iframe name="player-page" src="about:blank"></iframe>
)"sv,
		                   fmt::arg("style", style),
		                   fmt::arg("plugins", plugins),
		                   fmt::arg("version", mgps::version::ui));
	}

	static svg_test const svg_tests_plugin_info[] = {
	    {with_plugins(
	         ", with help from <span class='module'>[unknown]</span>."),
	     {},
	     {{}}},
	    {with_plugins(
	         ", with help from <span class='module'>module-name</span>."),
	     {},
	     {{"module-name"}}},
	    {with_plugins(", with help from <span class='module' "
	                  "title=\"helpfull string\">module-name</span>."),
	     {},
	     {{"module-name", "helpfull string"}}},
	    {with_plugins(", with help from <span class='module' "
	                  "title=\"helpfull string\">module-name<small> "
	                  "(v1)</small></span>."),
	     {},
	     {{"module-name", "helpfull string", "v1"}}},
	    {with_plugins(
	         ", with help from <span class='module'>module-name<small> "
	         "(v1)</small></span>."),
	     {},
	     {{"module-name", {}, "v1"}}},
	    {with_plugins(R"(, with help from:
<ul>
    <li><span class='module'>[unknown]</span></li>
    <li><span class='module'>&gt;&gt;module-name&lt;&lt;</span></li>
</ul>
)"),
	     {},
	     {{}, {">>module-name<<"}}},
	    {with_plugins(R"(, with help from:
<ul>
    <li><span class='module'>Hamlet</span><br/><i><small>Something &quot;fishy&quot;</small></i></li>
    <li><span class='module'>Skolov &amp; Joplin<small> (3.14)</small></span></li>
</ul>
)"),
	     {},
	     {{"Hamlet", "Something \"fishy\""}, {"Skolov & Joplin", {}, "3.14"}}},
	};

	inline std::string make_output(std::string_view content = {}) {
		return fmt::format(R"(<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
{style}
<body>{content}<div class='versions'>Generated using <span class='module'>mGPS<small> ({version})</small></span></div>
<iframe name="player-page" src="about:blank"></iframe>
)"sv,
		                   fmt::arg("style", style),
		                   fmt::arg("content", content),
		                   fmt::arg("version", mgps::version::ui));
	}

	static svg_test const svg_tests_trace[] = {
	    {make_output()},
	    {make_output(one_empty_trip), {mgps::trip{}}},
	    {make_output(one_empty_trip_with_date), {mgps::trip{local_days{2000_y/mar/14} + 12h + 34min + 45s}}},
	    {make_output(two_empty_trips), {mgps::trip{}, mgps::trip{}}},
	    {make_output(trip_1line_1deg_north),
	     {
	         mgps::trip{local_days{2000_y/mar/14} + 12h + 34min + 45s,
	                    {},
	                    trace{{{}, 1000s},
	                          {polyline{{{}, 1000s},
	                                    {pt(0_N, 0_E, {100}),
	                                     pt(.1_N, 0_E, {100}, 1000s)}}}},
	                    nullptr},
	     }},
	    {make_output(trip_2line_2degs_north_south),
	     {
	         mgps::trip{local_days{2000_y/mar/14} + 12h + 34min + 45s,
	                    {},
	                    trace{
	                        {{}, 2000s},
	                        {
	                            polyline{{{}, 1000s},
	                                     {
	                                         pt(0_N, 0_E, {100}),
	                                         pt(.1_N, 0_E, {100}, 1000s),
	                                     }},
	                            polyline{{{}, 1000s},
	                                     {
	                                         pt(.11_N, .01_E, {100}, 1100s),
	                                         pt(.11_N, .11_E, {100}, 2100s),
	                                     }},
	                        },
	                    },
	                    nullptr},
	     }},
	};

	INSTANTIATE_TEST_SUITE_P(Trace,
	                         HtmlSvg,
	                         ::testing::ValuesIn(svg_tests_trace));
	INSTANTIATE_TEST_SUITE_P(PluginList,
	                         HtmlSvg,
	                         ::testing::ValuesIn(svg_tests_plugin_info));
}  // namespace svg::testing
