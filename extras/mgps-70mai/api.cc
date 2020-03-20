#include <api.hh>
#include <cmath>
#include <filesystem>

namespace fs = std::filesystem;

namespace mgps::plugin::mai::api {
	namespace {
		struct file_reader_data {
			std::chrono::milliseconds duration{
			    std::chrono::milliseconds::max()};
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

	bool probe(char const* filename, isom::fs_data* fs) {
		auto info = isom::mai::get_filename_info(filename);
		if (info.type == clip::unrecognized) return false;

		auto bits = fs->open(filename);
		if ((!bits->valid()) || (!bits->is_isom())) return false;

		return true;
	}

	bool load(char const* filename,
	          mgps::plugin::file_info* out,
	          isom::fs_data* fs) {
		if (!out) return false;

		using namespace isom::mai;

		auto const info = get_filename_info(filename);
		if (info.type == clip::unrecognized) return false;
		return load(filename, info.type, info.ts.time_since_epoch(), out, fs);
	}

	bool load(char const* filename,
	          clip force_type,
	          std::chrono::milliseconds force_ts,
	          file_info* out,
	          isom::fs_data* fs) {
		if (!out) return false;

		using namespace isom;
		using namespace isom::mai;

		auto bits = fs->open(filename);
		if ((!bits->valid()) || (!bits->is_isom())) return false;

		file_reader_data data{};

		auto const size = bits->seek_end();
		if (!read_box<file_reader>(*bits, {0u, size, box_type::UNKNOWN}, data))
			return false;

		if (data.duration <= std::chrono::milliseconds::zero() ||
		    data.duration == std::chrono::milliseconds::max())
			return false;

		auto new_end =
		    std::remove_if(std::begin(data.points), std::end(data.points),
		                   [](gps_point& in) { return !in.valid; });
		auto const new_size = static_cast<size_t>(
		    std::distance(std::begin(data.points), new_end));

		out->set_clip(force_type);
		out->set_filename(filename);
		out->set_timestamp(force_ts);
		out->set_duration(data.duration);

		out->before_points(new_size);
		std::for_each(std::begin(data.points), new_end, [&](gps_point& in) {
			out->append_point(
			    {in.lat, in.lon},
			    track::speed_cast<track::speed_km>(in.metres_per_hour), in.pos,
			    std::chrono::seconds{1});
		});
		return true;
	}
}  // namespace mgps::plugin::mai::api
