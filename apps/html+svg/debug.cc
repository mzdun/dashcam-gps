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

std::string duration_from(std::chrono::milliseconds time) {
	using namespace std::chrono;
	using namespace date;

	if (time == milliseconds::max()) return "Unknown";

	auto ms = time;
	auto const h = floor<hours>(ms);
	auto const m = floor<minutes>(ms) - h;
	auto const s = floor<seconds>(ms) - m - h;
	ms -= (s + m + h);

	if (h == hours{0}) {
		return fmt::format("{:02}:{:02}.{:03}",
		                   static_cast<unsigned>(m.count()),
		                   static_cast<unsigned>(s.count()),
		                   static_cast<unsigned>(ms.count()));
	}

	if (h < days{1}) {
		return fmt::format(
		    "{:02}:{:02}:{:02}.{:03}", static_cast<unsigned>(h.count()),
		    static_cast<unsigned>(m.count()), static_cast<unsigned>(s.count()),
		    static_cast<unsigned>(ms.count()));
	}

	auto const d = floor<days>(h);

	return fmt::format(
	    "{} days, {:02}:{:02}:{:02}.{:03}", static_cast<unsigned>(d.count()),
	    static_cast<unsigned>((h - d).count()),
	    static_cast<unsigned>(m.count()), static_cast<unsigned>(s.count()),
	    static_cast<unsigned>(ms.count()));
}

std::string duration_from(std::chrono::seconds time) {
	using namespace std::chrono;
	using namespace date;

	if (time == duration_cast<seconds>(milliseconds::max())) return "Unknown";

	auto s = time;
	auto const h = floor<hours>(s);
	auto const m = floor<minutes>(s) - h;
	s -= (m + h);

	if (h == hours{0}) {
		return fmt::format("{:02}:{:02}", static_cast<unsigned>(m.count()),
		                   static_cast<unsigned>(s.count()));
	}

	if (h < days{1}) {
		return fmt::format(
		    "{:02}:{:02}:{:02}", static_cast<unsigned>(h.count()),
		    static_cast<unsigned>(m.count()), static_cast<unsigned>(s.count()));
	}

	auto const d = floor<days>(h);

	return fmt::format(
	    "{} days, {:02}:{:02}:{:02}", static_cast<unsigned>(d.count()),
	    static_cast<unsigned>((h - d).count()),
	    static_cast<unsigned>(m.count()), static_cast<unsigned>(s.count()));
}

std::string duration_from(std::chrono::minutes time) {
	using namespace std::chrono;
	using namespace date;

	auto const hms = hh_mm_ss{time};

	return fmt::format("{:02}:{:02}",
	                   static_cast<unsigned>(hms.hours().count()),
	                   static_cast<unsigned>(hms.minutes().count()));
}

std::string coord_from(mgps::track::coordinate const& pos) {
	return fmt::format("{}*{:02}.{:03}{}", pos.degrees(), pos.minutes(),
	                   pos.thousandths_of_a_minute(), to_char(pos.direction));
}
