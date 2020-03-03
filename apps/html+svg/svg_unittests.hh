#pragma once

#include <fmt/format.h>
#include <string>
#include <string_view>
#include "mgps/version.hh"

namespace svg::testing {
	using namespace std::literals;

	constexpr auto style = R"(<style>
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

</style>)"sv;

	constexpr auto one_empty_trip = R"(
<h2>00:00</h2>
<h3>Information</h3>

<table class="info">
<tr><th>Duration:</th><td>00:00</td></tr>
<tr><th>Playlist:</th><td>0 clips</td></tr>
<tr><th>Plot:</th><td>0 lines with 0 points</td></tr>
<tr><th>Distance:</th><td>0.0 km</td></tr>
</table>

<h3>Playlist</h3>

<table class="clips">
<thead>
<tr><th>Start</th><th>Duration</th><th>Clip</th></tr>
</thead>
</td></tr>
</table>
)"sv;

	constexpr auto one_empty_trip_with_date = R"(
<h1>2000-03-14</h1>
<h2>12:34</h2>
<h3>Information</h3>

<table class="info">
<tr><th>Duration:</th><td>00:00</td></tr>
<tr><th>Playlist:</th><td>0 clips</td></tr>
<tr><th>Plot:</th><td>0 lines with 0 points</td></tr>
<tr><th>Distance:</th><td>0.0 km</td></tr>
</table>

<h3>Playlist</h3>

<table class="clips">
<thead>
<tr><th>Start</th><th>Duration</th><th>Clip</th></tr>
</thead>
</td></tr>
</table>
)"sv;

	inline auto two_empty_trips = fmt::format("{0}{0}", one_empty_trip);

	constexpr auto trip_1line_1deg_north = R"(
<h1>2000-03-14</h1>
<h2>12:34</h2>

<div class="route">
<svg width="3.003003003003003cm" height="25.525525525525527cm" viewBox=" 0 0 2000.0 17000.0" xmlns="http://www.w3.org/2000/svg" style="stroke:#255fb2;stroke-width:60;stroke-linejoin:round;stroke-linecap:round;fill:none">
<rect x="0" y="0" width="2000.0" height="17000.0" rx="600" style="fill:#FEF7E0;stroke:none"/>
<path style="stroke:white;stroke-width:180" d="M1000 16000 L1000 1000 " />
<circle style="stroke:white;stroke-width:180" cx="1000.0" cy="16000.0" r="40" />
<g>
<path d="M1000 16000 L1000 1000 " >
<title>11.12 km, 16:40.000 (40 km/h avg)</title>
</path><circle style="stroke-width:60" cx="1000.0" cy="16000.0" r="40" />
</g>
</svg>
</div>

<h3>Information</h3>

<table class="info">
<tr><th>Duration:</th><td>00:00</td></tr>
<tr><th>Playlist:</th><td>0 clips</td></tr>
<tr><th>Plot:</th><td>1 line with 2 points</td></tr>
<tr><th>Distance:</th><td>11.12 km</td></tr>
<tr><th>Average speed:</th><td>40 km/h</td></tr>
</table>

<h3>Playlist</h3>

<table class="clips">
<thead>
<tr><th>Start</th><th>Duration</th><th>Clip</th></tr>
</thead>
</td></tr>
</table>
)"sv;

	constexpr auto trip_2line_2degs_north_south = R"(
<h1>2000-03-14</h1>
<h2>12:34</h2>

<div class="route">
<svg width="19.51951951951952cm" height="27.777777777777782cm" viewBox=" 0 0 13000.0 18500.000000000004" xmlns="http://www.w3.org/2000/svg" style="stroke:#255fb2;stroke-width:60;stroke-linejoin:round;stroke-linecap:round;fill:none">
<rect x="0" y="0" width="13000.0" height="18500.000000000004" rx="600" style="fill:#FEF7E0;stroke:none"/>
<path style="stroke:white;stroke-width:180" d="M1000 17500 L1000 2500 " />
<circle style="stroke:white;stroke-width:180" cx="1000.0" cy="17500.000000000004" r="40" />
<path style="stroke:white;stroke-width:180" d="M2000 1000 L12000 1000 " />
<circle style="stroke:white;stroke-width:180" cx="2000.0" cy="1000.0000000000036" r="40" />
<g>
<path d="M1000 17500 L1000 2500 " >
<title>11.12 km, 16:40.000 (40 km/h avg)</title>
</path><circle style="stroke-width:60" cx="1000.0" cy="17500.000000000004" r="40" />
</g>
<g>
<path d="M2000 1000 L12000 1000 " >
<title>11.119 km, 16:40.000 (40 km/h avg)</title>
</path><circle style="stroke-width:60" cx="2000.0" cy="1000.0000000000036" r="40" />
</g>
</svg>
</div>

<h3>Information</h3>

<table class="info">
<tr><th>Duration:</th><td>00:00</td></tr>
<tr><th>Playlist:</th><td>0 clips</td></tr>
<tr><th>Plot:</th><td>2 lines with 4 points</td></tr>
<tr><th>Distance:</th><td>22.239 km</td></tr>
<tr><th>Average speed:</th><td>40 km/h</td></tr>
</table>

<h3>Playlist</h3>

<table class="clips">
<thead>
<tr><th>Start</th><th>Duration</th><th>Clip</th></tr>
</thead>
</td></tr>
</table>
)"sv;
}  // namespace svg::testing
