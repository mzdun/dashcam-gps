#include <cmath>
#include <mgps-70mai/plugin.hh>
#include <mgps/cstdio.hh>
#include <mgps/track/trace.hh>
#include <mgps/trip.hh>
#include <mgps/video/playlist.hh>
#include <numeric>

namespace fs = std::filesystem;

namespace mgps::mai::api {
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
	}  // namespace

	bool probe(char const* filename) {
		auto info = isom::mai::get_filename_info(filename);
		if (info.type == clip::unrecognized) return false;

		auto bits = isom::cstdio::open(filename);
		if ((!bits.valid()) || (!bits.is_isom())) return false;

		return true;
	}

	bool load(char const* filename, media_file* out) {
		if (!out) return false;

		using namespace isom;
		using namespace isom::mai;

		auto info = get_filename_info(filename);
		if (info.type == clip::unrecognized) return false;

		auto bits = cstdio::open(filename);
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
}  // namespace mgps::mai::api

namespace mgps::mai {
	bool plugin::probe(char const* filename) const {
		return api::probe(filename);
	}

	bool plugin::load(char const* filename, media_file* out) const {
		return api::load(filename, out);
	}
}  // namespace mgps::mai
