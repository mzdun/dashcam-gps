#pragma once

#include <iostream>
#include <mgps/track/coordinate.hh>

std::ostream& operator<<(std::ostream& dbg, date::local_days const& time);
std::ostream& operator<<(std::ostream& dbg,
                         mgps::local_ms const& time);
std::ostream& operator<<(std::ostream& dbg,
                         std::chrono::milliseconds const& time);
std::ostream& operator<<(std::ostream& dbg, std::chrono::seconds const& time);
std::ostream& operator<<(std::ostream& dbg, std::chrono::minutes const& time);
std::ostream& operator<<(std::ostream& dbg, mgps::track::coordinate const& pos);

template <typename Counter>
struct pl {
	Counter count;
	const char* label;
	pl(Counter count, const char* label) : count{count}, label{label} {}
};

template <typename Counter>
inline std::ostream& operator<<(std::ostream& dbg, pl<Counter> const& val) {
	dbg << val.count << ' ' << val.label;
	if (val.count != 1) dbg << 's';
	return dbg;
}
