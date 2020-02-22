#pragma once

#include <fmt/format.h>

#include <iostream>
#include <mgps/track/coordinate.hh>

std::string date_time_from(date::local_days time);
std::string date_time_from(mgps::local_ms time);
std::string duration_from(std::chrono::milliseconds time);
std::string duration_from(std::chrono::seconds time);
std::string duration_from(std::chrono::minutes time);

template <typename Counter>
struct pl {
	Counter count;
	const char* label;
	pl(Counter count, const char* label) : count{count}, label{label} {}
};

template <typename Duration>
struct date_time {
	date::local_time<Duration> time;
	date_time(date::local_time<Duration> time) : time{time} {}
};

template <typename Rep, typename Period>
struct dur {
	std::chrono::duration<Rep, Period> time;
	dur(std::chrono::duration<Rep, Period> time) : time{time} {}
};

namespace fmt {
	template <typename Counter>
	struct formatter<pl<Counter>> : formatter<string_view> {
		template <typename FormatContext>
		auto format(pl<Counter> const& val, FormatContext& ctx) {
			auto s = fmt::format("{} {}{}", val.count, val.label,
			                     val.count != 1 ? "s" : "");
			return formatter<string_view>::format(s, ctx);
		}
	};

	template <typename Rep, typename Period>
	struct formatter<dur<Rep, Period>> : formatter<string_view> {
		template <typename FormatContext>
		auto format(dur<Rep, Period> const& val, FormatContext& ctx) {
			auto s = duration_from(val.time);
			return formatter<string_view>::format(s, ctx);
		}
	};

	template <typename Duration>
	struct formatter<date_time<Duration>> : formatter<string_view> {
		template <typename FormatContext>
		auto format(date_time<Duration> const& val, FormatContext& ctx) {
			auto s = date_time_from(val.time);
			return formatter<string_view>::format(s, ctx);
		}
	};
}  // namespace fmt
