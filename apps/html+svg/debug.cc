#include "debug.hh"

#include <cinttypes>

std::string date_time_from(date::local_days time) {
	using namespace date;
	using namespace std::chrono;
	auto const ymd = year_month_day{time};

	return fmt::format("{}-{:02}-{:02}", static_cast<int>(ymd.year()),
	                   static_cast<unsigned>(ymd.month()),
	                   static_cast<unsigned>(ymd.day()));
}

std::string date_time_from(mgps::local_ms time) {
	using namespace date;
	using namespace std::chrono;
	auto dayz = floor<days>(time);
	auto secs = time - local_seconds{dayz};

	auto const ymd = year_month_day{dayz};
	auto const hms = hh_mm_ss{secs};

	return fmt::format(
	    "{}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}", static_cast<int>(ymd.year()),
	    static_cast<unsigned>(ymd.month()), static_cast<unsigned>(ymd.day()),
	    static_cast<unsigned>(hms.hours().count()),
	    static_cast<unsigned>(hms.minutes().count()),
	    static_cast<unsigned>(hms.seconds().count()),
	    static_cast<unsigned>(hms.subseconds().count()));
}

auto subseconds(date::hh_mm_ss<std::chrono::seconds>) {
	return std::chrono::seconds{0};
}

auto subseconds(date::hh_mm_ss<std::chrono::milliseconds> hmss) {
	return hmss.subseconds();
}

template <typename Duration>
class duration_from_impl {
public:
	duration_from_impl(Duration dur) : is_undefined_{dur == max_}, hms_{dur} {}

	std::string format(std::string_view const (&formats)[3]) const {
		if (is_undefined_) return "Unknown";
		return fmt::format(formats[ndx_], hms_.is_negative() ? "-" : "",
		                   d_.count(), (hms_.hours() - d_).count(),
		                   hms_.minutes().count(), hms_.seconds().count(),
		                   subseconds(hms_).count());
	}

private:
	static constexpr Duration max_ =
	    std::chrono::duration_cast<Duration>(std::chrono::milliseconds::max());
	bool is_undefined_;
	date::hh_mm_ss<Duration> hms_;
	date::days d_{date::floor<date::days>(hms_.hours())};
	size_t ndx_ = hms_.hours() == std::chrono::hours{0}
	                  ? 0
	                  : hms_.hours() < date::days{1} ? 1 : 2;
};

std::string duration_from(std::chrono::milliseconds time) {
	using namespace std::literals;

	static constexpr std::string_view formats[] = {
	    "{0}{3:02}:{4:02}.{5:03}"sv, "{0}{2:02}:{3:02}:{4:02}.{5:03}"sv,
	    "{0}{1} days, {2:02}:{3:02}:{4:02}.{5:03}"sv};

	return duration_from_impl{time}.format(formats);
}

std::string duration_from(std::chrono::seconds time) {
	using namespace std::literals;

	static constexpr std::string_view formats[] = {
	    "{0}{3:02}:{4:02}"sv, "{0}{2:02}:{3:02}:{4:02}"sv,
	    "{0}{1} days, {2:02}:{3:02}:{4:02}"sv};

	return duration_from_impl{time}.format(formats);
}

std::string duration_from(std::chrono::minutes time) {
	using namespace std::chrono;
	using namespace date;

	if (time == duration_cast<minutes>(milliseconds::max())) return "Unknown";

	auto const hms = hh_mm_ss{time};

	return fmt::format("{}{:02}:{:02}", hms.is_negative() ? "-" : "",
	                   hms.hours().count(), hms.minutes().count());
}
