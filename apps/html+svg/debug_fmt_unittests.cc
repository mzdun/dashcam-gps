#include <fmt/ostream.h>
#include <gtest/gtest.h>

#include "debug.hh"

namespace std {
	inline void PrintTo(string_view const& s, ::std::ostream* os) {
		testing::internal::PrintStringTo({s.data(), s.size()}, os);
	}
}  // namespace std

namespace mgps::svg::testing {
	using ::testing::TestWithParam;
	using ::testing::ValuesIn;

	using namespace std::chrono;
	using namespace std::literals;
	using namespace date;
	using namespace date::literals;

	struct dur_test {
		milliseconds input;
		std::string_view expected_ms;
		std::string_view expected_s;
		std::string_view expected_min;
	};

	struct date_time_test {
		milliseconds input;
		std::string_view expected;
	};

	static void PrintTo(dur_test const& param, std::ostream* os) {
		fmt::print(*os, R"([{}ms -> "{}"/"{}"/"{}"])", param.input.count(),
		           param.expected_ms, param.expected_s, param.expected_min);
	}

	static void PrintTo(date_time_test const& param, std::ostream* os) {
		fmt::print(*os, R"([{}ms -> "{}"])", param.input.count(),
		           param.expected);
	}

	struct FmtDuration : TestWithParam<dur_test> {};
	struct FmtDateTime : TestWithParam<date_time_test> {};

	TEST_P(FmtDuration, Millis) {
		auto const& param = GetParam();
		auto const actual = fmt::format("{}", dur{param.input});
		ASSERT_EQ(param.expected_ms, actual);
	}

	TEST_P(FmtDuration, Secs) {
		auto const& param = GetParam();
		auto const actual =
		    fmt::format("{}", dur{duration_cast<seconds>(param.input)});
		ASSERT_EQ(param.expected_s, actual);
	}

	TEST_P(FmtDuration, Mins) {
		auto const& param = GetParam();
		auto const actual =
		    fmt::format("{}", dur{duration_cast<minutes>(param.input)});
		ASSERT_EQ(param.expected_min, actual);
	}

	TEST_P(FmtDateTime, Millis) {
		auto const& param = GetParam();
		auto const actual = fmt::format("{}", date_time{local_ms{param.input}});
		ASSERT_EQ(param.expected, actual);
	}

	TEST_P(FmtDateTime, Days) {
		auto const& param = GetParam();
		auto const actual = fmt::format(
		    "{}", date_time{local_days{date::floor<date::days>(param.input)}});
		auto const expected =
		    param.expected.substr(0, param.expected.find(' '));
		ASSERT_EQ(expected, actual);
	}

	constexpr dur_test mk(milliseconds const ms,
	                      std::string_view const expected_ms,
	                      std::string_view const expected_min) {
		return {ms, expected_ms,
		        expected_ms.substr(0, expected_ms.length() - 4), expected_min};
	}

	constexpr dur_test dur_tests[] = {
	    mk(0s, "00:00.000"sv, "00:00"sv),
	    mk(1234ms, "00:01.234"sv, "00:00"sv),
	    mk(1234567ms, "20:34.567"sv, "00:20"sv),
	    mk(12345678910ms, "142 days, 21:21:18.910"sv, "3429:21"sv),
	    mk(-2h - 24min - 17s - 139ms, "-02:24:17.139"sv, "-02:24"sv),
	    {milliseconds::max(), "Unknown"sv, "Unknown"sv, "Unknown"sv}};

	INSTANTIATE_TEST_SUITE_P(Tests,
	                         FmtDuration,
	                         ::testing::ValuesIn(dur_tests));

	constexpr date_time_test date_time_tests[] = {
	    {-63712700580s, "-49-01-10 11:37:00.000"sv},
	    {local_days{2_d / jun / 455}.time_since_epoch(), "455-06-02 00:00:00.000"sv},
	    {0s, "1970-01-01 00:00:00.000"sv},
	    {1234ms, "1970-01-01 00:00:01.234"sv},
	    {1234567ms, "1970-01-01 00:20:34.567"sv},
	    {12345678910ms, "1970-05-23 21:21:18.910"sv},
	    {-2h - 24min - 17s - 139ms, "1969-12-31 21:35:42.861"sv},
	};

	INSTANTIATE_TEST_SUITE_P(Tests,
	                         FmtDateTime,
	                         ::testing::ValuesIn(date_time_tests));
}  // namespace mgps::svg::testing
