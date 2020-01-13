#include "svg.hh"

#include <cctype>
#include <cinttypes>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mgps-70mai/plugin.hh>
#include <mgps/cstdio.hh>
#include <mgps/trip.hh>
#include <optional>

#include "debug.hh"

namespace fs = std::filesystem;

std::ostream& operator<<(std::ostream& out, mgps::clip clip_type) {
	using namespace mgps;
	out << "<span class=\"clip-type\"";
	switch (clip_type) {
		case clip::emergency:
			out << " title=\"Emergency\"";
			break;
		case clip::parking:
			out << " title=\"Parking incident\"";
			break;
		default:
			break;
	}
	out << ">&#x1F3AC";
	if (clip_type != clip::normal) {
		out << "<span class=\"overlay\">";
		switch (clip_type) {
			case clip::emergency:
				out << "&#x1F198;";
				break;
			case clip::parking:
				out << "&#x1F17F;&#xFE0F;";
				break;
			default:
				out << "&#x1F615;";
				break;
		}
		out << "</span>";  // overlay
	}
	return out << "</span>";  // clip-type
}

namespace mgps::svg {
	using namespace mgps::isom;
	using hourf = ch::duration<double, ch::hours::period>;

	struct lat_lon {
		double lat{};
		double lon{};

		template <size_t id>
		auto& get() noexcept {
			if constexpr (id == 0)
				return lon;
			else if constexpr (id == 1)
				return lat;
		}

		template <size_t id>
		auto const& get() const noexcept {
			if constexpr (id == 0)
				return lon;
			else if constexpr (id == 1)
				return lat;
		}
	};

	struct projected {
		double y{};
		double x{};

		template <size_t id>
		auto& get() noexcept {
			if constexpr (id == 0)
				return x;
			else if constexpr (id == 1)
				return y;
		}

		template <size_t id>
		auto const& get() const noexcept {
			if constexpr (id == 0)
				return x;
			else if constexpr (id == 1)
				return y;
		}
	};

	class projection {
	public:
		projection(lat_lon const& center) : center_{center} {};
		projected project(lat_lon const& pt) const noexcept {
			auto const lat = center_.lat + (pt.lat - center_.lat) * 3 / 2;
			return {lat, pt.lon};
		}
		projected project(track::point const& pt) const noexcept {
			return project(lat_lon{pt.lat.as_float(), pt.lon.as_float()});
		}

	private:
		static double deg2rad(double angle) noexcept {
			return angle * 3.141592653589793 / 180.0;
		};

		lat_lon center_;
	};

	void svg_trace(std::ostream& out, track::trace const& trace) {
		static constexpr auto MARGIN = 0.01;

		if (!trace.has_points()) return;
		auto projection_center = [&] {
			auto projection_bounds =
			    trace.boundary_box_impl<lat_lon>([](track::point const& pt) {
				    return lat_lon{pt.lat.as_float(), pt.lon.as_float()};
			    });
			return projection_bounds.center();
		}();

		projection camera{projection_center};
		auto bounds =
		    trace.boundary_box_impl<projected>([&](track::point const& pt) {
			    return camera.project(
			        lat_lon{pt.lat.as_float(), pt.lon.as_float()});
		    });

		bounds.topLeft.y += MARGIN;
		bounds.topLeft.x -= MARGIN;

		bounds.bottomRight.y -= MARGIN;
		bounds.bottomRight.x += MARGIN;

		auto const scale = 100000;
		auto const cm = scale * 2 / 300;
		auto const height = bounds.height() * scale;
		auto const width = bounds.width() * scale;
		out << R"(<div class="route">
<svg width=")"
		    << width / cm << R"(cm" height=")" << height / cm
		    << R"(cm" viewBox=" 0 0 )" << width << " " << height
		    << R"(" xmlns="http://www.w3.org/2000/svg" style="stroke:#255fb2;stroke-width:60;stroke-linejoin:round;stroke-linecap:round;fill:none">
<rect x="0" y="0" width=")"
		    << width << R"(" height=")" << height
		    << R"(" rx="600" style="fill:#FEF7E0;stroke:none"/>
)";
		for (auto& line : trace.lines) {
			bool first_point = true;
			out << R"(<path style="stroke:white;stroke-width:180" d=")";
			for (auto const& gps : line.points) {
				auto const pt = camera.project(gps);
				if (first_point) {
					out << "M";
					first_point = false;
				} else {
					out << "L";
				}
				out << (pt.x - bounds.topLeft.x) * scale << ' '
				    << height - (pt.y - bounds.bottomRight.y) * scale << ' ';
			}
			out << R"(" />
)";
			if (!line.points.empty()) {
				auto const& pt = camera.project(line.points.front());
				out << R"(<circle style="stroke:white;stroke-width:180" cx=")"
				    << (pt.x - bounds.topLeft.x) * scale << R"(" cy=")"
				    << height - (pt.y - bounds.bottomRight.y) * scale
				    << R"(" r="40" />
)";
			}
		}

		for (auto& line : trace.lines) {
			auto const distance = double(line.distance()) / 1000.0;
			out << "<g>\n";
			bool first_point = true;
			out << R"(<path d=")";
			for (auto const& gps : line.points) {
				auto const pt = camera.project(gps);
				if (first_point) {
					out << "M";
					first_point = false;
				} else {
					out << "L";
				}
				out << (pt.x - bounds.topLeft.x) * scale << ' '
				    << height - (pt.y - bounds.bottomRight.y) * scale << ' ';
			}
			out << R"(" >
<title>)" << distance
			    << " km, " << line.duration << " ("
			    << int(distance / hourf{line.duration}.count() + .5)
			    << R"( km/h avg)</title>
</path>)";
			if (!line.points.empty()) {
				auto const& pt = camera.project(line.points.front());
				out << R"(<circle style="stroke-width:60" cx=")"
				    << (pt.x - bounds.topLeft.x) * scale << R"(" cy=")"
				    << height - (pt.y - bounds.bottomRight.y) * scale
				    << R"(" r="40" />
)";
			}
			out << "</g>\n";
		}
		out << R"(</svg>
</div>
)";
	}

	void trace_trip(std::ostream& out, mgps::trip const& trip) {
		size_t gpses{};
		for (auto& line : trip.trace.lines)
			gpses += line.points.size();
		ch::milliseconds driven{};
		uint64_t dist{};
		for (auto& line : trip.trace.lines) {
			driven += line.duration;
			dist += line.distance();
		}

		auto const start_hour = trip.start - floor<date::days>(trip.start);

		out << R"(
<h2>)" << floor<ch::minutes>(start_hour)
		    << R"(</h2>

)";
		svg_trace(out, trip.trace);

		out << R"(

<h3>Information</h3>

<table class="info">
<tr><th>Duration:</th><td>)"
		    << floor<ch::seconds>(trip.playlist.duration)
		    << "</td></tr>\n<tr><th>Playlist:</th><td>"
		    << pl{trip.playlist.media.size(), "clip"}
		    << "</td></tr>\n<tr><th>Plot:</tthd><td>"
		    << pl{trip.trace.lines.size(), "line"} << " with "
		    << pl{gpses, "point"} << "</td></tr>\n<tr><th>Distance:</th><td>"
		    << double(dist) / 1000.0 << " km";
		if (driven.count())
			out << "</td></tr>\n<tr><th>Average speed:</th><td>"
			    << int((double(dist) / 1000.0) / hourf{driven}.count() + 0.5)
			    << " km/h";
		out << R"(</td></tr>
</table>

<h3>Playlist</h3>

<table class="clips">
<thead>
<tr><th>Start</th><th>Duration</th><th>Clip</th></tr>
</thead>
)";

		bool has_millis_in_offset = false;
		for (auto& file : trip.playlist.media) {
			if (floor<ch::seconds>(file.offset) != file.offset) {
				has_millis_in_offset = true;
				break;
			}
		}

		std::error_code ec{};
		auto cwd = fs::current_path(ec);
		auto can_use_proximate = !ec;

		for (auto& clip : trip.playlist.media) {
			out << "<tr><td class=\"num\">";
			if (has_millis_in_offset)
				out << clip.offset.time_since_epoch();
			else
				out << floor<ch::seconds>(clip.offset).time_since_epoch();
			out << "</td><td class=\"num\">" << clip.duration << "</td><td>";

			auto file = trip.footage(clip);
			if (file) {
				out << R"(<a target="player-page" href=")";
				bool printed_proximate = false;
				if (can_use_proximate) {
					auto relative = fs::proximate(file->filename, cwd, ec);
					if (!ec) {
						out << relative.string();
						printed_proximate = true;
					}
				}
				if (!printed_proximate) out << file->filename.string();
				out << R"(">)" << file->type << "</a>";
			} else {
				out << "&#x2049;&#xFE0F;";
			}
			out << "</td></tr>\n";
		}

		out << R"(</td></tr>
</table>

)";
	}
	void html_trace(std::ostream& out, std::vector<mgps::trip> const& trips) {
		using namespace std::chrono;
		using namespace date;

		out << R"(<!DOCTYPE html>
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

g:hover { stroke: #F88 }

a { text-decoration: none }


.num {
	text-align: right;
}

.clip-type {
	font-size: 120%
}
.overlay {
	font-size: 75%;
	margin-left: -1em;
	vertical-align: sub;
}

</style>
<body>

)";

		date::local_days prev_date{};
		for (auto const& trip : trips) {
			auto const date = floor<days>(trip.start);

			if (date != prev_date) {
				prev_date = date;
				out << "<h1>" << date << "</h1>\n";
			}
			trace_trip(out, trip);
		}
		out << R"(
<iframe name="player-page" src="about:blank"></iframe>
)";
	}
}  // namespace mgps::svg
