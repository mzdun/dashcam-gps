#include <charconv>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#include <ctre.hpp>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include <mgps-70mai/70mai.hh>
#include <mgps/isom.hh>
#include <mgps/track/point.hh>

namespace mgps::isom::mai {
	namespace {
		static constexpr auto filenames = ctll::fixed_string{
		    "^"
		    "([A-Za-z]{2})"                   // NO, EV, PA
		    "([0-9]{4})([0-9]{2})([0-9]{2})"  // YYYYMMDD
		    "-"
		    "([0-9]{2})([0-9]{2})([0-9]{2})"  // HHMMSS
		    "-.*$"};

		template <typename Number>
		bool from_chars(std::string_view chars, Number& value) {
			auto const end = chars.data() + chars.size();
			auto const result = std::from_chars(chars.data(), end, value);
			return result.ec == std::errc{} && result.ptr == end;
		}

		track::NESW NESW(char c) {
			switch (c) {
				case 'N':
				case 'n':
					return track::NESW::N;
				case 'E':
				case 'e':
					return track::NESW::E;
				case 'S':
				case 's':
					return track::NESW::S;
				case 'W':
				case 'w':
					return track::NESW::W;
				default:
					break;
			}
			using Int = std::underlying_type_t<track::NESW>;
			return static_cast<track::NESW>(std::numeric_limits<Int>::max());
		}

		track::coordinate make_coord(uint32_t deg_min_1000, char direction) {
			auto const full_degrees =
			    (deg_min_1000 / 100'000ull) * track::coordinate::precision::den;
			auto const minutes_fraction = (deg_min_1000 % 100'000ull) *
			                              track::coordinate::precision::den /
			                              60'000;
			return {full_degrees + minutes_fraction, NESW(direction)};
		}

		struct seek_to {
			uint64_t pos;
			storage& stg;
			~seek_to() { stg.seek(pos); }
		};

		bool next_point(storage& data, gps_point& pt) {
			seek_to end{data.tell() + 36, data};

			if (data.eof()) return false;

			auto const has_record = data.get<uint32_t>();
			if (!has_record) return false;

			if (data.eof()) return false;
			auto const has_gps = data.get<uint32_t>();

			if (data.eof()) return false;
			auto const seconds = std::chrono::seconds{data.get<uint32_t>()};

			if (!has_gps) {
				pt = gps_point{};
				return true;
			}

			if (data.eof()) return false;
			auto const kmph = track::speed{(data.get<uint32_t>() + 500) / 1000};

			if (data.eof()) return false;
			auto const lat_dir = data.get<char>();
			if (data.eof()) return false;
			auto const latitude = make_coord(data.get<uint32_t>(), lat_dir);

			if (data.eof()) return false;
			auto const lon_dir = data.get<char>();
			if (data.eof()) return false;
			auto const longitude = make_coord(data.get<uint32_t>(), lon_dir);

			if (!latitude.valid() || !longitude.valid()) return false;

			pt.lat = latitude;
			pt.lon = longitude;
			pt.pos = seconds;
			pt.kmph = kmph;
			pt.valid = true;
			return true;
		}

		namespace loaders {
			// duration
			LEAF_BOX_READER(mvhd_box, duration_type) {
				range_storage bits{&full, box};
				bits.seek(0);
				auto const version = bits.get<std::uint8_t>();
				bits.get<std::uint8_t>();
				bits.get<std::uint8_t>();
				bits.get<std::uint8_t>();

				if (bits.eof()) return false;

				auto const short_ints = version == 0;
				if (!short_ints && version > 1) {
					data() = duration_type{};
					return false;
				}

				if (short_ints) {
					bits.get<std::uint32_t>();
					bits.get<std::uint32_t>();
				} else {
					bits.get<std::uint64_t>();
					bits.get<std::uint64_t>();
				}

				if (bits.eof()) return false;
				data().timescale = bits.get_u32();

				if (bits.eof()) return false;
				if (short_ints) {
					data().length = bits.get_u32();
				} else {
					data().length = bits.get_u64();
				};

				return true;
			}

			TREE_BOX_READER(moov_box, duration_type) {
				switch (box.type) {
					case box_type::mvhd:
						return read_box<mvhd_box>(full, box, data());
					default:
						break;
				}
				return true;
			}
		}  // namespace loaders
	}      // namespace

	clip_filename_info get_filename_info(std::string_view filename) {
		auto parts = ctre::match<filenames>(filename);
		if (!parts) return {};

		auto const clip_type = [](std::string_view type) {
#define UP(C) std::toupper(static_cast<unsigned char>(C))
#define TYPE_IS(L1, L2) ((UP(type[0]) == L1) && (UP(type[1]) == L2))
			if (TYPE_IS('N', 'O')) return clip::normal;
			if (TYPE_IS('E', 'V')) return clip::emergency;
			if (TYPE_IS('P', 'A')) return clip::parking;
#undef TYPE_IS
#undef UP
			return clip::other;
		}(parts.get<1>().to_view());

		unsigned uY{}, uM{}, uD{}, h{}, m{}, s{};
		if (!from_chars(parts.get<2>().to_view(), uY)) return {};
		if (!from_chars(parts.get<3>().to_view(), uM)) return {};
		if (!from_chars(parts.get<4>().to_view(), uD)) return {};
		if (!from_chars(parts.get<5>().to_view(), h)) return {};
		if (!from_chars(parts.get<6>().to_view(), m)) return {};
		if (!from_chars(parts.get<7>().to_view(), s)) return {};

		using namespace date;
		using namespace std::chrono;
		auto const Y = year{static_cast<int>(uY)};
		auto const M = static_cast<int>(uM);
		auto const D = static_cast<int>(uD);
		auto const date = Y / M / D;
		if (!date.ok() || h > 23 || m > 59 || s > 59) return {};
		auto const ts = local_days{date} + hours{h} + minutes{m} + seconds{s};
		return {clip_type, ts};
	}

	bool read_GPS_box(storage& full,
	                  box_info const& box,
	                  std::vector<gps_point>& out) {
		std::vector<gps_point> points(1);
		if (box.type != GPS_box) return false;

		range_storage bits{&full, box};
		while (next_point(bits, points.back())) {
			if (!points.back().valid) continue;
			points.push_back({});
		}
		if (!points.empty() && !points.back().valid) points.pop_back();
		out = std::move(points);
		return true;
	}

	bool read_moov_mhdr_duration(storage& full,
	                             box_info const& box,
	                             std::chrono::milliseconds& out) {
		if (box.type != box_type::moov) return false;

		duration_type duration;
		if (!read_box<loaders::moov_box>(full, box, duration)) return false;

		out = duration.to_chrono();
		return true;
	}
}  // namespace mgps::isom::mai
