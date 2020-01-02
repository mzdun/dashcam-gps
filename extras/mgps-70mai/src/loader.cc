#include "mgps-70mai/loader.hh"

#include <algorithm>
#include <cmath>
#include <mgps/cstdio.hh>

namespace mgps::library::mai {
	namespace {
		struct file_reader_data {
			std::chrono::milliseconds duration;
			std::vector<isom::mai::gps_point> points;
		};

		TREE_BOX_READER(file_reader, file_reader_data) {
			using namespace isom::mai;
			switch (box.type) {
				case box_type::moov:
					return read_moov_mhdr_duration(full, box, data().duration);
				case GPS_box:
					return read_GPS_box(full, box, data().points);
				default:
					break;
			}
			return true;
		}

		template <typename FileInfo>
		void copy_strides(std::vector<std::vector<FileInfo>>& interim_trip,
		                  library::trip& trip) {
			trip.strides.reserve(interim_trip.size());

			for (auto& interim_stride : interim_trip) {
				trip.strides.emplace_back();
				auto& stride = trip.strides.back();

				stride.clips.reserve(interim_stride.size());

				for (auto& interim_clip : interim_stride) {
					stride.clips.emplace_back();
					auto& clip = stride.clips.back();

					// here, "offset" is not yet the offset from the start of
					// the stride, but the time and date taken from file name;
					// this will change, as we will start bubbling the offset
					// towards the trip

					clip.offset = interim_clip.date_time.time_since_epoch();
					clip.duration = interim_clip.duration;
					clip.filename = std::move(interim_clip.filename);

					// same here: for now, a point's time is it's absolute time
					// within local calendar and it will be later relative to
					// the trip's date
					for (auto& point : interim_clip.points)
						point.time += clip.offset;
				}

				// the calendar information is propagated to the stride,
				// while clips are getting relative to the stride
				auto const stride_offset = stride.clips.front().offset;
				for (auto& clip : stride.clips)
					clip.offset -= stride_offset;
				auto const& last_clip = stride.clips.back();

				stride.offset = stride_offset;
				stride.duration = last_clip.offset + last_clip.duration;
			}

			// the calendar information is propagated to the trip, while strides
			// and points are getting relative to the trip
			auto const trip_offset = trip.strides.front().offset;
			for (auto& stride : trip.strides)
				stride.offset -= trip_offset;
			auto const& last_stride = trip.strides.back();

			// and we are finally home, with trip holding calendar info and
			// everything else being relative to its parent
			trip.start = local_milliseconds{trip_offset};
			trip.duration = last_stride.offset + last_stride.duration;
		}

		bool detached(track::gps_data const& previous,
		              track::gps_data const& current) {
			return (current.time - previous.time) > ch::seconds{1};
		}

		template <typename FileInfo>
		void copy_points(std::vector<std::vector<FileInfo>>& interim_trip,
		                 library::trip& trip) {
			{
				size_t reserve_for_segments{};
				track::gps_data const* previous = nullptr;

				for (auto& interim_stride : interim_trip) {
					for (auto& interim_clip : interim_stride) {
						for (auto& point : interim_clip.points) {
							if (!previous || detached(*previous, point)) {
								++reserve_for_segments;
							}
							previous = &point;
						}
					}
				}

				trip.plot.segments.reserve(reserve_for_segments);
			}

			{
				size_t reserve_for_segment{};
				track::gps_data const* previous = nullptr;

				for (auto& interim_stride : interim_trip) {
					for (auto& interim_clip : interim_stride) {
						for (auto& point : interim_clip.points) {
							if (previous && detached(*previous, point)) {
								trip.plot.segments.emplace_back();
								trip.plot.segments.back().points.reserve(
								    reserve_for_segment);
								reserve_for_segment = 0;
							}
							previous = &point;
							++reserve_for_segment;
						}
					}
				}

				if (reserve_for_segment) {
					trip.plot.segments.emplace_back();
					trip.plot.segments.back().points.reserve(
					    reserve_for_segment);
				}
			}
			{
				size_t segment_index{};
				track::gps_data const* previous = nullptr;

				for (auto& interim_stride : interim_trip) {
					for (auto& interim_clip : interim_stride) {
						for (auto& point : interim_clip.points) {
							if (previous && detached(*previous, point)) {
								++segment_index;
							}
							previous = &point;

							auto& segment =
							    trip.plot.segments[segment_index].points;
							if (segment.empty() ||
							    segment.back().time != point.time)
								segment.push_back(point);
							else
								segment.back() = point;
						}
					}
				}
			}

			for (auto& segment : trip.plot.segments) {
				segment.offset = segment.points.front().time;
				for (auto& point : segment.points)
					point.time -= segment.offset;
				segment.duration = segment.points.back().time;
			}

			if (trip.plot.segments.empty()) {
				trip.plot.offset = trip.start.time_since_epoch();
				trip.plot.duration = ch::milliseconds{};
			} else {
				trip.plot.offset = trip.plot.segments.front().offset;
				for (auto& segment : trip.plot.segments)
					segment.offset -= trip.plot.offset;
				auto& last_segment = trip.plot.segments.back();
				trip.plot.duration =
				    last_segment.offset + last_segment.duration;
			}
		}
	};  // namespace

	bool loader::add_file(fs::path const& file_name) noexcept {
		using namespace isom;
		using namespace isom::mai;

		auto info = get_filename_info(file_name.filename().string());
		if (info.type == clip::unrecognized) return false;

		auto bits = cstdio::open(file_name.string().c_str());
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

		std::vector<track::gps_data> points;
		points.reserve(new_size);
		std::transform(std::begin(data.points), new_end,
		               std::back_inserter(points),
		               [](gps_point& in) -> track::gps_data {
			               return {{in.lat, in.lon}, in.kmph, in.pos};
		               });
		interim_.push_back(
		    {file_name, info.type, info.ts, data.duration, std::move(points)});

		return true;
	}

	void loader::add_directory(fs::path const& dir_name) noexcept {
		std::error_code ec{};

		auto dirname = fs::path{dir_name};
		auto items = fs::directory_iterator{dirname, ec};
		if (ec) return;

		for (auto const& entry : items) {
			if (!entry.is_regular_file(ec) || ec) continue;
			add_file(entry.path());
		}
	}

	std::vector<trip> loader::build(ch::milliseconds max_stride_gap) {
		std::sort(begin(interim_), end(interim_),
		          [](auto const& lhs, auto const& rhs) {
			          return lhs.date_time < rhs.date_time;
		          });

		local_milliseconds prev{};
		for (auto& file : interim_) {
			if (prev > file.date_time) file.date_time = prev;
			prev = file.date_time + file.duration;
		}

		prev = local_milliseconds{};

		// vector - vector - vector - file_info
		// library - trip - stride - clip

		std::vector<std::vector<std::vector<file_info>>> interim_library{};
		for (auto& file : interim_) {
			if ((prev + max_stride_gap) < file.date_time) {
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

			copy_strides(interim_trip, trip);
			copy_points(interim_trip, trip);
		}

		interim_.clear();
		return library;
	}
}  // namespace mgps::library::mai
