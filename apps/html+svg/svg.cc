#include "svg.hh"

#include <fmt/ostream.h>

#include <cctype>
#include <cinttypes>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mgps/api.hh>
#include <mgps/library.hh>
#include <mgps/trip.hh>
#include <optional>

#include "debug.hh"

namespace fs = std::filesystem;

struct relative {
	std::string const& filename;
	std::filesystem::path* pcwd{};
};

namespace fmt {
	template <>
	struct formatter<relative> : formatter<string_view> {
		template <typename FormatContext>
		auto format(relative const& rel, FormatContext& ctx) {
			if (rel.pcwd) {
				std::error_code ec{};
				auto r = fs::proximate(rel.filename, *rel.pcwd, ec);
				if (!ec) {
					return formatter<string_view>::format(r.string(), ctx);
				}
			}
			return formatter<string_view>::format(rel.filename, ctx);
		}
	};

	template <>
	struct formatter<mgps::clip> : formatter<string_view> {
		template <typename FormatContext>
		auto format(mgps::clip clip_type, FormatContext& ctx) {
			using namespace mgps;
			std::string out = "<span class=\"clip-type\"";
			switch (clip_type) {
				case clip::emergency:
					out += " title=\"Emergency\"";
					break;
				case clip::parking:
					out += " title=\"Parking incident\"";
					break;
				default:
					break;
			}
			out += ">&#x1F3AC";
			if (clip_type != clip::normal) {
				out += "<span class=\"overlay\">";
				switch (clip_type) {
					case clip::emergency:
						out += "&#x1F198;";
						break;
					case clip::parking:
						out += "&#x1F17F;&#xFE0F;";
						break;
					default:
						out += "&#x1F615;";
						break;
				}
				out += "</span>";  // overlay
			}
			out += "</span>";  // clip-type
			return formatter<string_view>::format(out, ctx);
		}
	};
}  // namespace fmt

namespace mgps::svg {
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
		fmt::print(out, R"(

<div class="route">
<svg width="{0}cm" height="{1}cm" viewBox=" 0 0 {2} {3}" xmlns="http://www.w3.org/2000/svg" style="stroke:#255fb2;stroke-width:60;stroke-linejoin:round;stroke-linecap:round;fill:none">
<rect x="0" y="0" width="{2}" height="{3}" rx="600" style="fill:#FEF7E0;stroke:none"/>
)",
		           width / cm, height / cm, width, height);
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
				fmt::print(
				    out,
				    R"(<circle style="stroke:white;stroke-width:180" cx="{}" cy="{}" r="40" />
)",
				    (pt.x - bounds.topLeft.x) * scale,
				    height - (pt.y - bounds.bottomRight.y) * scale);
			}
		}

		for (auto& line : trace.lines) {
			auto const distance = double(track::distance(&line)) / 1000.0;
			bool first_point = true;
			out << R"(<g>
<path d=")";
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
			fmt::print(out, R"(" >
<title>{} km, {} ({} km/h avg)</title>
</path>)",
			           distance, dur{line.duration},
			           line.duration.count() ? int(distance / hourf{line.duration}.count() + .5) : 0);

			if (!line.points.empty()) {
				auto const& pt = camera.project(line.points.front());
				fmt::print(
				    out,
				    R"(<circle style="stroke-width:60" cx="{}" cy="{}" r="40" />
)",
				    (pt.x - bounds.topLeft.x) * scale,
				    height - (pt.y - bounds.bottomRight.y) * scale);
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
			dist += track::distance(&line);
		}

		auto const start_hour =
		    trip.start - date::floor<date::days>(trip.start);

		fmt::print(out, R"(
<h2>{}</h2>)",
		           dur{date::floor<ch::minutes>(start_hour)});

		svg_trace(out, trip.trace);

		fmt::print(out, R"(
<h3>Information</h3>

<table class="info">
<tr><th>Duration:</th><td>{0}</td></tr>
<tr><th>Playlist:</th><td>{1}</td></tr>
<tr><th>Plot:</th><td>{2} with {3}</td></tr>
<tr><th>Distance:</th><td>{4} km)",
		           dur{date::floor<ch::seconds>(trip.playlist.duration)},
		           pl{trip.playlist.media.size(), "clip"},
		           pl{trip.trace.lines.size(), "line"}, pl{gpses, "point"},
		           double(dist) / 1000.0);

		if (driven.count()) {
			fmt::print(
			    out, R"(</td></tr>
<tr><th>Average speed:</th><td>{} km/h)",
			    int((double(dist) / 1000.0) / hourf{driven}.count() + 0.5));
		}

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
			if (date::floor<ch::seconds>(file.offset) != file.offset) {
				has_millis_in_offset = true;
				break;
			}
		}

		std::error_code ec{};
		auto cwd = fs::current_path(ec);
		auto pcwd = !ec ? &cwd : nullptr;

		constexpr char format[] =
		    "<tr><td class=\"num\">{}</td><td class=\"num\">{}</td><td>";
		for (auto& clip : trip.playlist.media) {
			if (has_millis_in_offset) {
				fmt::print(out, format, dur{clip.offset.time_since_epoch()},
				           dur{clip.duration});
			} else {
				fmt::print(out, format,
				           dur{date::floor<ch::seconds>(clip.offset)
				                   .time_since_epoch()},
				           dur{clip.duration});
			}

			auto file = footage(&trip, clip);
			if (file) {
				fmt::print(out, R"(<a target="player-page" href="{}">{}</a>)",
				           relative{file->filename, pcwd}, file->type);
			} else {
				out << "&#x2049;&#xFE0F;";
			}
			out << "</td></tr>\n";
		}

		out << R"(</td></tr>
</table>
)";
	}

	struct xmlize {
		std::string_view str;
		friend std::ostream& operator<<(std::ostream& o, xmlize const& in) {
			for (auto c : in.str) {
				switch (c) {
					default:
						o << c;
						break;
					case '&':
						o << "&amp;";
						break;
					case '<':
						o << "&lt;";
						break;
					case '>':
						o << "&gt;";
						break;
					case '"':
						o << "&quot;";
						break;
				}
			}
			return o;
		}
	};
	void html_trace(std::ostream& out,
	                mgps::library const& lib,
	                std::vector<mgps::trip> const& trips) {
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
	bottom: 60pt;
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

.versions {
	margin: 2em auto;
	padding: 0 3em;
	display: table;
	font-size: small;
	color: #ccc;
}

.versions .module {
	font-family: monospace, monospace;
	color: #aaa;
}

.versions ul {
	margin: 0;
	padding: 0;
	list-style: none;
}

.versions ul li {
	padding-left: 24px;
}

.versions ul li:before {
	padding-right: 6px;
	margin-left: -10px;
	content: "-";
}

</style>
<body>)";

		date::local_days prev_date{};
		for (auto const& trip : trips) {
			auto const date = floor<days>(trip.start);

			if (date != prev_date) {
				prev_date = date;
				fmt::print(out, "\n<h1>{}</h1>", date);
			}
			trace_trip(out, trip);
		}
		fmt::print(out, R"(<div class='versions'>Generated using <span class='module'>mGPS<small> ({})</small></span>)",
		           get_version().str.ui);
		auto& plugins = lib.plugins();
		if (!plugins.empty()) {
			if (plugins.size() == 1) {
				auto& ptr = plugins.front();
				auto& info = ptr->info();

				auto const name = info.name.empty()
				                      ? "[unknown]"
				                      : fmt::format("{}", xmlize{info.name});
				std::string version{}, descr{};
				if (!info.version.empty()) {
					version = fmt::format(R"(<small> ({})</small>)",
					                      xmlize{info.version});
				}

				if (!info.description.empty()) {
					descr =
					    fmt::format(R"( title="{}")", xmlize{info.description});
				}

				fmt::print(
				    out, ", with help from <span class='module'{}>{}{}</span>.",
				    descr, name, version);
			} else {
				out << ", with help from:\n<ul>";

				for (auto& ptr : plugins) {
					auto& info = ptr->info();

					auto const name =
					    info.name.empty()
					        ? "[unknown]"
					        : fmt::format("{}", xmlize{info.name});
					std::string version{}, descr{};
					if (!info.version.empty()) {
						version = fmt::format(R"(<small> ({})</small>)",
						                      xmlize{info.version});
					}

					if (!info.description.empty()) {
						descr = fmt::format("<br/><i><small>{}</small></i>",
						                    xmlize{info.description});
					}

					fmt::print(out, R"(
    <li><span class='module'>{}{}</span>{}</li>)",
					           name, version, descr);
				}

				out << "\n</ul>\n";
			}
		}
		out << R"(</div>
<iframe name="player-page" src="about:blank"></iframe>
)";
	}
}  // namespace mgps::svg
