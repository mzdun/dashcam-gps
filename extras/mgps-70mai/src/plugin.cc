#include <cmath>
#include <mgps-70mai/plugin.hh>
#include <mgps/cstdio.hh>
#include <mgps/track/trace.hh>
#include <mgps/trip.hh>
#include <mgps/video/playlist.hh>
#include <numeric>

namespace fs = std::filesystem;

namespace mgps::mai {
	namespace {
		struct file_reader_data {
			std::chrono::milliseconds duration;
			std::vector<isom::mai::gps_point> points;
		};

		TREE_BOX_READER(file_reader, file_reader_data) {
			using namespace isom::mai;
			if (box.type == box_type::moov)
				return read_moov_mhdr_duration(full, box, data().duration);
			if (box.type == GPS_box)
				return read_GPS_box(full, box, data().points);
			return true;
		}

#if 0
		local_ms copy_chunks(
		    std::vector<std::vector<file>>& interim_trip,
		    video::playlist& playlist) {
			playlist.clips.reserve(std::accumulate(
			    begin(interim_trip), end(interim_trip), size_t{},
			    [](size_t prev, auto const& interim_chunk) {
				    return interim_chunk.size() + prev;
			    }));

			playback_ms travel{};
			playback_ms playback{};
			auto const travel_start =
			    [](std::vector<std::vector<file>>& interim_trip) {
				    for (auto const& interim_chunk : interim_trip) {
					    if (interim_chunk.empty()) continue;
					    return interim_chunk.front().date_time;
				    }
				    return local_ms{};
			    }(interim_trip);

			for (auto& interim_chunk : interim_trip) {
				for (auto& interim_clip : interim_chunk) {
					playlist.clips.emplace_back();
					auto& clip = playlist.clips.back();

					playback += interim_clip.duration;
					travel = playback_ms{interim_clip.date_time - travel_start};

					clip.offset = playback_ms{travel.time_since_epoch()};
					clip.duration = interim_clip.duration;
					clip.type = interim_clip.type;
					clip.filename = std::move(interim_clip.filename);

					for (auto& point : interim_clip.points)
						point.offset += clip.offset.time_since_epoch();
				}
			}

			playlist.duration = playback.time_since_epoch();
			return travel_start;
		}

		bool detached(track::gps_point const& previous,
		              track::gps_point const& current) {
			return (current.offset - previous.offset) > ch::seconds{1};
		}

		void copy_points(std::vector<std::vector<file>>& interim_trip,
		                 track::trace& trace,
		                 playback_ms const& start) {
			{
				size_t reserve_for_lines{};
				track::gps_point const* previous = nullptr;

				for (auto& interim_chunk : interim_trip) {
					for (auto& interim_clip : interim_chunk) {
						for (auto& point : interim_clip.points) {
							if (!previous || detached(*previous, point)) {
								++reserve_for_lines;
							}
							previous = &point;
						}
					}
				}

				trace.lines.reserve(reserve_for_lines);
			}

			{
				size_t reserve_for_line{};
				track::gps_point const* previous = nullptr;

				for (auto& interim_chunk : interim_trip) {
					for (auto& interim_clip : interim_chunk) {
						for (auto& point : interim_clip.points) {
							if (previous && detached(*previous, point)) {
								trace.lines.emplace_back();
								trace.lines.back().points.reserve(
								    reserve_for_line);
								reserve_for_line = 0;
							}
							previous = &point;
							++reserve_for_line;
						}
					}
				}

				if (reserve_for_line) {
					trace.lines.emplace_back();
					trace.lines.back().points.reserve(reserve_for_line);
				}
			}
			{
				size_t line_index{};
				track::gps_point const* previous = nullptr;

				for (auto& interim_chunk : interim_trip) {
					for (auto& interim_clip : interim_chunk) {
						for (auto& point : interim_clip.points) {
							if (previous && detached(*previous, point)) {
								++line_index;
							}
							previous = &point;

							auto& line = trace.lines[line_index].points;
							if (line.empty() ||
							    line.back().offset != point.offset)
								line.push_back(point);
							else
								line.back() = point;
						}
					}
				}
			}

			for (auto& line : trace.lines) {
				using namespace std::literals;
				line.offset = playback_ms{line.points.front().offset};
				line.duration =
				    line.points.empty() ? 0ms : (line.points.size() - 1) * 1s;
				for (auto& point : line.points)
					point.offset -= line.offset.time_since_epoch();
			}

			if (trace.lines.empty()) {
				trace.offset = start;
				trace.duration = ch::milliseconds{};
			} else {
				trace.offset = trace.lines.front().offset;
				for (auto& line : trace.lines)
					line.offset -= trace.offset.time_since_epoch();
				auto& last_line = trace.lines.back();
				trace.duration =
				    last_line.offset.time_since_epoch() + last_line.duration;
			}
		}
#endif
	}  // namespace

	bool plugin::probe(fs::path const& filename) const {
		auto info = isom::mai::get_filename_info(filename.filename().string());
		if (info.type == clip::unrecognized) return false;

		auto bits = isom::cstdio::open(filename.string().c_str());
		if ((!bits.valid()) || (!bits.is_isom())) return false;

		return true;
	}

	bool plugin::load(fs::path const& filename, media_file* out) const {
		if (!out) return false;

		using namespace isom;
		using namespace isom::mai;

		auto info = get_filename_info(filename.filename().string());
		if (info.type == clip::unrecognized) return false;

		auto bits = cstdio::open(filename.string().c_str());
		if ((!bits.valid()) || (!bits.is_isom())) return false;

		file_reader_data data{};

		auto const size = bits.seek_end();
		if (!read_box<file_reader>(bits, {0u, size, box_type::UNKNOWN}, data))
			return false;

		auto new_end =
		    std::remove_if(std::begin(data.points), std::end(data.points),
		                   [](gps_point& in) { return !in.valid; });
		auto const new_size = static_cast<size_t>(
		    std::distance(std::begin(data.points), new_end));

		out->filename = filename;
		out->type = info.type;
		out->date_time = info.ts;
		out->duration = data.duration;

		out->points.reserve(new_size);
		std::transform(std::begin(data.points), new_end,
		               std::back_inserter(out->points),
		               [](gps_point& in) -> track::gps_point {
			               return {{in.lat, in.lon}, in.kmph, in.pos};
		               });
		return true;
	}

#if 0
	std::vector<trip> loader::build(ch::milliseconds max_chunk_gap) {
		std::sort(begin(interim_), end(interim_),
		          [](auto const& lhs, auto const& rhs) {
			          return lhs.date_time < rhs.date_time;
		          });

		local_ms prev{};
		for (auto& file : interim_) {
			if (prev > file.date_time) file.date_time = prev;
			prev = file.date_time + file.duration;
		}

		prev = local_ms{};

		// vector - vector - vector - file_info
		// library - trip - chunk - clip

		std::vector<std::vector<std::vector<file>>> interim_library{};
		for (auto& file : interim_) {
			if ((prev + max_chunk_gap) < file.date_time) {
				interim_library.emplace_back(1);
			} else if (prev < file.date_time) {
				interim_library.back().emplace_back();
			}
			prev = file.date_time + file.duration;
			interim_library.back().back().emplace_back(std::move(file));
		}

		std::vector<trip> library;
		library.reserve(interim_library.size());
		for (auto& interim_trip : interim_library) {
			library.emplace_back();
			auto& trip = library.back();
			trip.owner = nullptr;

			trip.start = copy_chunks(interim_trip, trip.playlist);
			auto const travel_start = playback_ms{trip.start.time_since_epoch()};
			copy_points(interim_trip, trip.trace, travel_start);
		}

		interim_.clear();
		return library;
	}
#endif
}  // namespace mgps::mai
