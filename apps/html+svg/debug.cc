#include "debug.hh"

#include <cinttypes>

std::ostream& operator<<(std::ostream& dbg, date::local_days const& time) {
	using namespace date;
	using namespace std::chrono;
	auto const ymd = year_month_day{time};

	char buffer[1000];
	sprintf(buffer, "%d-%02u-%02u", static_cast<int>(ymd.year()),
	        static_cast<unsigned>(ymd.month()),
	        static_cast<unsigned>(ymd.day()));

	return dbg << buffer;
}

std::ostream& operator<<(std::ostream& dbg, mgps::local_ms const& time) {
	using namespace date;
	using namespace std::chrono;
	auto dayz = floor<days>(time);
	auto secs = time - local_seconds{dayz};

	auto const ymd = year_month_day{dayz};
	auto const hms = hh_mm_ss{secs};

	char buffer[1000];
	sprintf(buffer, "%d-%02u-%02u %02u:%02u:%02u.%03u",
	        static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()),
	        static_cast<unsigned>(ymd.day()),
	        static_cast<unsigned>(hms.hours().count()),
	        static_cast<unsigned>(hms.minutes().count()),
	        static_cast<unsigned>(hms.seconds().count()),
	        static_cast<unsigned>(hms.subseconds().count()));

	return dbg << buffer;
}

std::ostream& operator<<(std::ostream& dbg,
                         std::chrono::milliseconds const& time) {
	using namespace std::chrono;
	using namespace date;

	if (time == milliseconds::max()) return dbg << "Unknown";

	auto ms = time;
	auto const h = floor<hours>(ms);
	auto const m = floor<minutes>(ms) - h;
	auto const s = floor<seconds>(ms) - m - h;
	ms -= (s + m + h);

	char buffer[1000];
	if (h == hours{0}) {
		sprintf(buffer, "%02u:%02u.%03u", static_cast<unsigned>(m.count()),
		        static_cast<unsigned>(s.count()),
		        static_cast<unsigned>(ms.count()));
	} else if (h < days{1}) {
		sprintf(buffer, "%02u:%02u:%02u.%03u", static_cast<unsigned>(h.count()),
		        static_cast<unsigned>(m.count()),
		        static_cast<unsigned>(s.count()),
		        static_cast<unsigned>(ms.count()));
	} else {
		auto const d = floor<days>(h);

		sprintf(buffer, "%u days, %02u:%02u:%02u.%03u",
		        static_cast<unsigned>(d.count()),
		        static_cast<unsigned>((h - d).count()),
		        static_cast<unsigned>(m.count()),
		        static_cast<unsigned>(s.count()),
		        static_cast<unsigned>(ms.count()));
	}
	return dbg << buffer;
}

std::ostream& operator<<(std::ostream& dbg, std::chrono::seconds const& time) {
	using namespace std::chrono;
	using namespace date;

	if (time == milliseconds::max()) return dbg << "Unknown";

	auto s = time;
	auto const h = floor<hours>(s);
	auto const m = floor<minutes>(s) - h;
	s -= (m + h);

	char buffer[1000];
	if (h == hours{0}) {
		sprintf(buffer, "%02u:%02u", static_cast<unsigned>(m.count()),
		        static_cast<unsigned>(s.count()));
	} else if (h < days{1}) {
		sprintf(buffer, "%02u:%02u:%02u", static_cast<unsigned>(h.count()),
		        static_cast<unsigned>(m.count()),
		        static_cast<unsigned>(s.count()));
	} else {
		auto const d = floor<days>(h);

		sprintf(
		    buffer, "%u days, %02u:%02u:%02u", static_cast<unsigned>(d.count()),
		    static_cast<unsigned>((h - d).count()),
		    static_cast<unsigned>(m.count()), static_cast<unsigned>(s.count()));
	}
	return dbg << buffer;
}
std::ostream& operator<<(std::ostream& dbg, std::chrono::minutes const& time) {
	using namespace std::chrono;
	using namespace date;

	auto const hms = hh_mm_ss{time};

	char buffer[100];
	sprintf(buffer, "%02u:%02u", static_cast<unsigned>(hms.hours().count()),
	        static_cast<unsigned>(hms.minutes().count()));
	return dbg << buffer;
}

std::ostream& operator<<(std::ostream& dbg,
                         mgps::track::coordinate const& pos) {
	char buffer[200];
	sprintf(buffer, "%" PRIu64 "*%02" PRIu64 ".%03" PRIu64 "'%c", pos.degrees(),
	        pos.minutes(), pos.thousandths_of_a_minute(),
	        to_char(pos.direction));
	return dbg << buffer;
}
