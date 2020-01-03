#pragma once

#include <iostream>

#include "mgps-70mai/70mai.hh"

std::ostream& operator<<(std::ostream& dbg, date::local_days const& time);
std::ostream& operator<<(std::ostream& dbg,
                         mgps::local_milliseconds const& time);
std::ostream& operator<<(std::ostream& dbg,
                         std::chrono::milliseconds const& time);
std::ostream& operator<<(std::ostream& dbg, std::chrono::seconds const& time);
std::ostream& operator<<(std::ostream& dbg, std::chrono::minutes const& time);
std::ostream& operator<<(std::ostream& dbg,
                         mgps::library::track::coord const& pos);

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
