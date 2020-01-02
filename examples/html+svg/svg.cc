#include "svg.hh"

#include <cctype>
#include <cinttypes>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mgps-70mai/loader.hh>
#include <mgps/cstdio.hh>
#include <optional>

#include "debug.hh"

namespace fs = std::filesystem;

namespace mgps::svg {
	using namespace mgps::isom;
	using namespace mgps::library;

	void html_trace(std::vector<trip> const& trips) {
		using namespace std::chrono;
		using namespace date;

		std::cout << R"(<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<style>
body {
	font-family: sans-serif;
}

td {
	vertical-align: top;
}

h3 { margin-top: 0; }

iframe {
	position: fixed;
	bottom: 10pt;
	right: 10pt;
	width: 32vw;
	height: 16vw;
	border: none;
	background: black
}

g:hover { stroke: red }

a { text-decoration: none }


.num {
	text-align: right;
}
</style>
<body>

<table class="vignete">

)";

		date::local_days prev_date{};
		for (auto const& trip : trips) {
			auto const date = floor<days>(trip.start);
			auto const start_hour = trip.start - date;

			if (date != prev_date) {
				prev_date = date;
				std::cout << "<tr><td colspan=\"3\"><h1>" << date
				          << "</h1></td></tr>\n";
			}

			using hourf = ch::duration<double, ch::hours::period>;
			size_t clips{};
			for (auto& stride : trip.strides)
				clips += stride.clips.size();
			size_t gpses{};
			for (auto& seg : trip.plot.segments)
				gpses += seg.points.size();
			milliseconds driven{};
			uint64_t dist{};
			for (auto& segment : trip.plot.segments) {
				driven += segment.duration;
				dist += segment.distance();
			}

			std::cout << R"(
<tr><td colspan="3"><h2>)"
			          << floor<minutes>(start_hour) << R"(</h2></tr>

<tr><td>

<h3>Information</h3>

<table class="info">
<tr><th>Duration:</th><td>)"
					  << floor<seconds>(trip.duration)
			          << "</td></tr>\n<tr><th>Playback:</th><td>"
			          << pl{trip.strides.size(), "stride"} << " with "
			          << pl{clips, "clip"}
			          << "</td></tr>\n<tr><th>Plot:</tthd><td>"
			          << pl{trip.plot.segments.size(), "segment"} << " with "
			          << pl{gpses, "point"}
			          << "</td></tr>\n<tr><th>Distance:</th><td>"
			          << dist / 1000.0 << " km";
			if (driven.count())
				std::cout << "</td></tr>\n<tr><th>Average speed:</th><td>"
				          << int((dist / 1000.0) / hourf{driven}.count() + 0.5)
				          << " km/h";
			std::cout << R"(</td></tr>
</table>

</td><td>&nbsp;</td><td>

<h3>Movie strips</h3>

<table class="clips">
<thead>
<tr><th>Start</th><th>Duration</th><th>Count</th><th>Clips</th></tr>
</thead>
)";

			bool has_millis_in_offset = false;
			for (auto& stride : trip.strides) {
				if (floor<seconds>(stride.offset) != stride.offset) {
					has_millis_in_offset = true;
					break;
				}
			}
			for (auto& stride : trip.strides) {
				std::cout << "<tr><td class=\"num\">";
				if (has_millis_in_offset)
					std::cout << stride.offset;
				else
					std::cout << floor<seconds>(stride.offset);
				std::cout << "</td><td class=\"num\">" << stride.duration
				          << "</td><td class=\"num\">"
				          << pl{stride.clips.size(), "clip"} << "</td><td>";

				std::error_code ec{};
				auto cwd = fs::current_path(ec);
				auto can_use_proximate = !ec;
				auto clip_no = 0u;
				for (auto& clip : stride.clips) {
					std::cout << R"(<a target="player-page" href=")";
					bool printed_proximate = false;
					if (can_use_proximate) {
						auto relative = fs::proximate(clip.filename, cwd, ec);
						if (!ec) {
							std::cout << relative.string();
							printed_proximate = true;
						}
					}
					if (!printed_proximate) std::cout << clip.filename.string();
					std::cout << R"("><code>#)" << ++clip_no << "</code></a>\n";
				}
				std::cout << "</td></tr>\n";
			}

			std::cout << R"(</td></tr>
</table>

</td><td>&nbsp;</td></tr>
)";

			double hi_lat{}, hi_lon{}, lo_lat{}, lo_lon{};
			bool first_coord = true;

			for (auto& segment : trip.plot.segments) {
				for (auto& pt : segment.points) {
					auto lat = pt.lat.as_float();
					auto lon = pt.lon.as_float();
					if (first_coord) {
						first_coord = false;
						hi_lat = lo_lat = lat;
						hi_lon = lo_lon = lon;
						continue;
					}
					if (lat < lo_lat) lo_lat = lat;
					if (lat > hi_lat) hi_lat = lat;
					if (lon < lo_lon) lo_lon = lon;
					if (lon > hi_lon) hi_lon = lon;
				}
			}

			if (!first_coord) {
				lo_lat -= 0.005;
				lo_lon -= 0.005;
				hi_lat += 0.005;
				hi_lon += 0.005;
				auto const scale = 100000;
				auto const cm = scale * 3 / 400;
				auto const height = (hi_lat - lo_lat) * scale;
				auto const width = (hi_lon - lo_lon) * scale;
				std::cout
				    << R"(<tr><td colspan="4"><div class="route">
<svg width=")" << width / cm
				    << R"(cm" height=")" << height / cm
				    << R"(cm" viewBox=" 0 0 )" << width << " " << height
				    << R"(" xmlns="http://www.w3.org/2000/svg" style="stroke:#255fb2;stroke-width:60;fill:none">
)";
				for (auto& segment : trip.plot.segments) {
					auto const distance = segment.distance() / 1000.0;
					std::cout << "<g>\n";

					bool first_point = true;
					std::cout << R"(<path d=")";
					for (auto const& pt : segment.points) {
						if (first_point) {
							std::cout << "M";
							first_point = false;
						} else {
							std::cout << "L";
						}
						std::cout
						    << (pt.lon.as_float() - lo_lon) * scale << ' '
						    << height - (pt.lat.as_float() - lo_lat) * scale
						    << ' ';
					}
					std::cout
					    << R"(" >
<title>)" << distance << " km, "
					    << segment.duration << " ("
					    << int(distance / hourf{segment.duration}.count() + .5)
					    << R"( km/h avg)</title>
</path>)";
					if (!segment.points.empty()) {
						auto const& pt = segment.points.front();
						std::cout
						    << R"(<circle style="stroke-width:20" cx=")"
						    << (pt.lon.as_float() - lo_lon) * scale
						    << R"(" cy=")"
						    << height - (pt.lat.as_float() - lo_lat) * scale
						    << R"(" r="100" />
)";
					}
					std::cout << "</g>\n";
				}
				std::cout << R"(</svg>
</div>
</td></tr>
)";
			}
		}
		std::cout << R"(</table>
<iframe name="player-page" src="about:blank"></iframe>
)";
	}
}  // namespace mgps::svg
