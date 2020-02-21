#include <gtest/gtest.h>

#include <70mai.hh>

namespace mgps {
	static void PrintTo(clip type, std::ostream* os) {
		switch (type) {
			case clip::unrecognized:
				*os << "unrecognized";
				return;
			case clip::normal:
				*os << "normal";
				return;
			case clip::emergency:
				*os << "emergency";
				return;
			case clip::parking:
				*os << "parking";
				return;
			case clip::other:
				*os << "other";
				return;
			default:
				break;
		}
		*os << "clip(" << int(type) << ")";
	}
}  // namespace mgps

namespace mgps::plugin::isom::mai {
	static void PrintTo(clip_filename_info const& info, std::ostream* os) {
		*os << '{';
		PrintTo(info.type, os);
		*os << ", " << info.ts << ':' << info.ts.time_since_epoch().count()
		    << '}';
	}
}  // namespace mgps::plugin::isom::mai

namespace mgps::plugin::isom::mai::testing {

	struct info_param {
		std::string_view path;
		clip_filename_info expected;
	};

	static void PrintTo(info_param const& param, std::ostream* os) {
		*os << "[\"" << param.path << "\" -> ";
		PrintTo(param.expected, os);
		*os << "]";
	}

	struct NameTest : ::testing::TestWithParam<info_param> {};
	TEST_P(NameTest, Comparison) {
		auto const& param = GetParam();
		auto const actual = mai::get_filename_info(param.path);
		ASSERT_EQ(param.expected.type, actual.type);
		ASSERT_EQ(param.expected.ts, actual.ts);
	}

	using namespace std::literals;
	using namespace date;
	constexpr info_param mk(std::string_view path,
	                        clip type = clip::unrecognized,
	                        local_ms timestamp = {}) {
		return {path, {type, timestamp}};
	}
	constexpr info_param mk(std::string_view path,
	                        clip type,
	                        date::year_month_day const& ymd,
	                        std::chrono::milliseconds timeofday = {}) {
		return mk(path, type, local_days{ymd} + timeofday);
	}
	constexpr info_param filenames[] = {
	    mk("NO19700101-000000-000000.MP4", clip::normal, 1970_y / jan / 1),
	    mk("NO00000101-000000-", clip::normal, 0_y / jan / 1),
	    mk("NO19700101-153045-",
	       clip::normal,
	       1970_y / jan / 1,
	       15h + 30min + 45s),
	    mk("NO00000101-153045-blurblurbblurb",
	       clip::normal,
	       0_y / jan / 1,
	       15h + 30min + 45s),
	    mk("EV00000101-000000-000000.MP4", clip::emergency, 0_y / jan / 1),
	    mk("PA99991231-235959-...",
	       clip::parking,
	       9999_y / dec / 31,
	       23h + 59min + 59s),
	    mk("YY20380119-031407-...",
	       clip::other,
	       2038_y / jan / 19,
	       3h + 14min + 7s),
	    mk("ZZ20380119-031408-...",
	       clip::other,
	       2038_y / jan / 19,
	       3h + 14min + 8s),
	    mk("NO00000000-000000-000000.MP4", clip::unrecognized)};

	INSTANTIATE_TEST_SUITE_P(Filenames,
	                         NameTest,
	                         ::testing::ValuesIn(filenames));
}  // namespace mgps::plugin::isom::mai::testing
