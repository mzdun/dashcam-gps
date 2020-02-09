#include "listing.hh"

#include <cctype>
#include <cinttypes>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mgps/api.hh>
#include <mgps/library.hh>
#include <mgps/track/trace.hh>
#include <mgps/trip.hh>
#include <optional>

#include "debug.hh"

namespace fs = std::filesystem;
using namespace std::literals;

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

namespace mgps {
	namespace {
		using hourf = ch::duration<double, ch::hours::period>;
	}
}  // namespace mgps

namespace mgps {
	void write_trip(std::ostream& out,
	                mgps::trip const& trip,
	                library const& lib) {
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

#define TDr "<td style='text-align:right'>"
#define COLLAPSIBLE_START(lvl)          \
	"<div class='collapsible-content'>" \
	"<h" #lvl " class='collapsible-header'>"
#define SWITCH(lvl)                    \
	" <span class='switch'>[<a>"       \
	"<span class='shown'>Hide</span>"  \
	"<span class='hidden'>Show</span>" \
	"</a>]</span></h" #lvl             \
	">\n"                              \
	"<div class='content'>"

#define COLLAPSIBLE_END() "</div></div>"

		auto const print_row = [&](const char* name, auto&& value) {
			out << "<tr><th>" << name << "</th>" TDr << value << "</td></tr>\n";
		};

		out << "\n" COLLAPSIBLE_START(3) << date::floor<ch::minutes>(start_hour)
		    << SWITCH(3)
		    << "\n"
		       "<table>";
		print_row("Duration", date::floor<ch::seconds>(trip.playlist.duration));
		print_row("Playlist", pl{trip.playlist.media.size(), "clip"});
		out << "<tr><th>Plot</th><td style='text-align:right'>"
		    << pl{trip.trace.lines.size(), "ln"} << "<br/>" << pl{gpses, "pt"}
		    << "<br/>off: " << trip.trace.offset.time_since_epoch()
		    << "<br/>dur: " << trip.trace.duration << "</td><td>";

		if (!trip.trace.lines.empty()) {
			out << "\n"
			       "<table>"
			       "<thead>"
			       "<tr><th>Offset</th>"
			       "<th>Duration</th>"
			       "<th>Points</th>"
			       "<th>Distance</th>"
			       "<th>Avg speed</th></tr>"
			       "</thead>\n"
			       "<tbody>\n";

			for (auto const& line : trip.trace.lines) {
				auto distance = track::distance(&line);
				out << "<tr>" TDr << line.offset.time_since_epoch()
				    << "</td>" TDr << line.duration << "</td>" TDr
				    << line.points.size() << "</td>" TDr
				    << double(distance) / 1000.0 << " km</td>" TDr;
				if (line.duration.count())
					out << int((double(distance) / 1000.0) /
					               hourf{line.duration}.count() +
					           0.5);
				else
					out << "-";
				out << " km/h</td></tr>\n";
			}
			out << "</tbody></table>\n";
		}
		out << "</td></tr>\n";

		out << "<tr><th>Distance</th>" TDr << double(dist) / 1000.0
		    << " km</td></tr>\n"
		       "<tr><th>Average speed</th><td style='text-align:right'>";

		if (driven.count())
			out << int((double(dist) / 1000.0) / hourf{driven}.count() + 0.5);
		else
			out << "-";

		out << " km/h</td></tr>\n"
		       "</table>\n";

		/*for (auto const& line : trip.trace.lines) {
		    out << "\n##### " << line.offset.time_since_epoch() << " _("
		        << line.points.size()
		        << ")_\n"
		           "|Offset|Dist|Speed|\n"
		           "|-----:|-------:|----:|\n";

		    track::point const* prev = nullptr;
		    for (auto const& point : line.points) {
		        out << "|" << point.offset << "|";
		        if (prev) {
		            out << track::distance(*prev, point);
		        } else {
		            out << "-";
		        }
		        prev = &point;
		        out << " m|" << point.kmph << " km/h|\n";
		    }
		}*/

		out << "\n" COLLAPSIBLE_START(4) "Clips" SWITCH(4)
		    << "<table>"
		       "<thead>"
		       "<tr><th>Start</th>"
		       "<th>Duration</th>"
		       "<th>Ref</th>"
		       "<th>Pts</th>"
		       "<th>T</th>"
		       "<th>Path</th></tr></thead>\n"
		       "<tbody>\n";

		bool has_millis_in_offset = false;
		for (auto& file : trip.playlist.media) {
			if (date::floor<ch::seconds>(file.offset) != file.offset) {
				has_millis_in_offset = true;
				break;
			}
		}

		for (auto& file : trip.playlist.media) {
			out << "<tr>" TDr;
			if (has_millis_in_offset)
				out << file.offset.time_since_epoch();
			else
				out << date::floor<ch::seconds>(file.offset).time_since_epoch();
			out << "</td>" TDr << file.duration << "</td>" TDr << file.reference
			    << "</td>" TDr;
			auto ptr = lib.footage(file);
			if (!ptr)
				out << "-</td><td "
				       "style='text-align:center'>-</td><td>-</td></tr>\n";
			else
				out << ptr->points.size()
				    << "</td><td style='text-align:center'>" << ptr->type
				    << "</td><td>" << ptr->filename << "</td></tr>\n";
		}

		out << "</tbody>"
		       "</table>" COLLAPSIBLE_END() COLLAPSIBLE_END() "\n";
	}

	struct page_info {
		page kind;
		std::string_view title;
		ch::milliseconds gap{library::special_gap};
	};

	static constexpr page_info pages[] = {
	    {page::everything, "Everything"sv, library::default_gap},
	    {page::emergency, "Emergency"sv},
	    {page::parking, "Parking"sv},
	};

	void listing(std::ostream& out, library const& lib) {
		std::vector<trip> sections[std::size(pages)];

		out << R"(
<!DOCTYPE html>
<html>
<head>
<title>listing</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<style type="text/css">
/* GitHub stylesheet for MarkdownPad (http://markdownpad.com) */
/* Author: Nicolas Hery - http://nicolashery.com */
/* Version: b13fe65ca28d2e568c6ed5d7f06581183df8f2ff */
/* Source: https://github.com/nicolahery/markdownpad-github */

/* RESET
=============================================================================*/

html, body, div, span, applet, object, iframe, h1, h2, h3, h4, h5, h6, p, blockquote, pre, a, abbr, acronym, address, big, cite, code, del, dfn, em, img, ins, kbd, q, s, samp, small, strike, strong, sub, sup, tt, var, b, u, i, center, dl, dt, dd, ol, ul, li, fieldset, form, label, legend, table, caption, tbody, tfoot, thead, tr, th, td, article, aside, canvas, details, embed, figure, figcaption, footer, header, hgroup, menu, nav, output, ruby, section, summary, time, mark, audio, video {
  margin: 0;
  padding: 0;
  border: 0;
}

/* BODY
=============================================================================*/

body {
  font-family: Helvetica, arial, freesans, clean, sans-serif;
  font-size: 14px;
  line-height: 1.6;
  color: #333;
  background-color: #fff;
  padding: 20px;
  max-width: 960px;
  margin: 0 auto;
}

body>*:first-child {
  margin-top: 0 !important;
}

body>*:last-child {
  margin-bottom: 0 !important;
}

/* BLOCKS
=============================================================================*/

p, blockquote, ul, ol, dl, table, pre {
  margin: 15px 0;
}

/* HEADERS
=============================================================================*/

h1, h2, h3, h4, h5, h6 {
  margin: 20px 0 10px;
  padding: 0;
  font-weight: bold;
  -webkit-font-smoothing: antialiased;
}

h1 tt, h1 code, h2 tt, h2 code, h3 tt, h3 code, h4 tt, h4 code, h5 tt, h5 code, h6 tt, h6 code {
  font-size: inherit;
}

h1 {
  font-size: 28px;
  color: #000;
}

h2 {
  font-size: 24px;
  border-bottom: 1px solid #ccc;
  color: #000;
}

h3 {
  font-size: 18px;
}

h4 {
  font-size: 16px;
}

h5 {
  font-size: 14px;
}

h6 {
  color: #777;
  font-size: 14px;
}

body>h2:first-child, body>h1:first-child, body>h1:first-child+h2, body>h3:first-child, body>h4:first-child, body>h5:first-child, body>h6:first-child {
  margin-top: 0;
  padding-top: 0;
}

a:first-child h1, a:first-child h2, a:first-child h3, a:first-child h4, a:first-child h5, a:first-child h6 {
  margin-top: 0;
  padding-top: 0;
}

h1+p, h2+p, h3+p, h4+p, h5+p, h6+p {
  margin-top: 10px;
}

/* LINKS
=============================================================================*/

a {
  color: #4183C4;
  text-decoration: none;
}

a:hover {
  text-decoration: underline;
}

/* LISTS
=============================================================================*/

ul, ol {
  padding-left: 30px;
}

ul li > :first-child, 
ol li > :first-child, 
ul li ul:first-of-type, 
ol li ol:first-of-type, 
ul li ol:first-of-type, 
ol li ul:first-of-type {
  margin-top: 0px;
}

ul ul, ul ol, ol ol, ol ul {
  margin-bottom: 0;
}

dl {
  padding: 0;
}

dl dt {
  font-size: 14px;
  font-weight: bold;
  font-style: italic;
  padding: 0;
  margin: 15px 0 5px;
}

dl dt:first-child {
  padding: 0;
}

dl dt>:first-child {
  margin-top: 0px;
}

dl dt>:last-child {
  margin-bottom: 0px;
}

dl dd {
  margin: 0 0 15px;
  padding: 0 15px;
}

dl dd>:first-child {
  margin-top: 0px;
}

dl dd>:last-child {
  margin-bottom: 0px;
}

/* CODE
=============================================================================*/

pre, code, tt {
  font-size: 12px;
  font-family: Consolas, "Liberation Mono", Courier, monospace;
}

code, tt {
  margin: 0 0px;
  padding: 0px 0px;
  white-space: nowrap;
  border: 1px solid #eaeaea;
  background-color: #f8f8f8;
  border-radius: 3px;
}

pre>code {
  margin: 0;
  padding: 0;
  white-space: pre;
  border: none;
  background: transparent;
}

pre {
  background-color: #f8f8f8;
  border: 1px solid #ccc;
  font-size: 13px;
  line-height: 19px;
  overflow: auto;
  padding: 6px 10px;
  border-radius: 3px;
}

pre code, pre tt {
  background-color: transparent;
  border: none;
}

kbd {
    -moz-border-bottom-colors: none;
    -moz-border-left-colors: none;
    -moz-border-right-colors: none;
    -moz-border-top-colors: none;
    background-color: #DDDDDD;
    background-image: linear-gradient(#F1F1F1, #DDDDDD);
    background-repeat: repeat-x;
    border-color: #DDDDDD #CCCCCC #CCCCCC #DDDDDD;
    border-image: none;
    border-radius: 2px 2px 2px 2px;
    border-style: solid;
    border-width: 1px;
    font-family: "Helvetica Neue",Helvetica,Arial,sans-serif;
    line-height: 10px;
    padding: 1px 4px;
}

/* QUOTES
=============================================================================*/

blockquote {
  border-left: 4px solid #DDD;
  padding: 0 15px;
  color: #777;
}

blockquote>:first-child {
  margin-top: 0px;
}

blockquote>:last-child {
  margin-bottom: 0px;
}

/* HORIZONTAL RULES
=============================================================================*/

hr {
  clear: both;
  margin: 15px 0;
  height: 0px;
  overflow: hidden;
  border: none;
  background: transparent;
  border-bottom: 4px solid #ddd;
  padding: 0;
}

/* TABLES
=============================================================================*/

table th {
  font-weight: bold;
}

table th, table td {
  border: 1px solid #ccc;
  padding: 6px 13px;
  font-size: 14px;
}

table tr {
  border-top: 1px solid #ccc;
  background-color: #fff;
}

table tr:nth-child(2n) {
  background-color: #f8f8f8;
}

/* IMAGES
=============================================================================*/

img {
  max-width: 100%
}

/* APPLICATION
=============================================================================*/

.clip-type {
	font-size: 120%
}

.overlay {
	font-size: 75%;
	margin-left: -1em;
	vertical-align: sub;
}

.switch {
  font-size: smaller;
  font-weight: normal;
}
.switch a { cursor: pointer }

.collapsible-content.expanded > .collapsible-header .hidden,
.collapsible-content > .collapsible-header .shown,
.collapsible-content > .content {
  display: none
}

.collapsible-content.expanded > .collapsible-header .shown,
.collapsible-content > .collapsible-header .hidden {
  display: inline
}

.collapsible-content.expanded > .content {
  display: block
}
</style>
)";

		int id = 0;
		out << "<ol>";
		for (auto const& page : pages) {
			sections[id] = lib.build(page.kind, page.gap);
			auto const count = sections[id].size();
			++id;

			out << "<li>" << page.title << " <i>(" << count << ")</i></li>";
		}
		out << "</ol>\n";

		id = 0;
		for (auto const& trips : sections) {
			out << "<div class='section collapsible-content'><h1 "
			       "class='collapsible-header'>"
			    << pages[id].title << " <i>(" << trips.size()
			    << ")</i>" SWITCH(1);
			++id;

			date::local_days prev_date{};
			bool had_one = false;
			for (auto const& trip : trips) {
				auto const date = date::floor<date::days>(trip.start);

				if (date != prev_date) {
					prev_date = date;
					if (had_one) out << "</div>\n";
					had_one = true;
					out << "<div class='day'><h2>" << date << "</h2>\n";
				}
				write_trip(out, trip, lib);
			}
			if (had_one) out << "</div>";
			out << "</div>" COLLAPSIBLE_END();
		}

		out << R"(<script>
  window.onload = function () {
    let links = document
      .querySelectorAll(".collapsible-content .switch a")
      .forEach(link => {
        let a = link
        link.onclick = function(e) {
          let div = a.parentElement.parentElement.parentElement
          div.classList.toggle('expanded')
          e.stopImmediatePropagation()
        }
      })
  }
</script>
</body>
</html>)";
	}
}  // namespace mgps