#pragma once

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

#include <date/date.h>

#include <filesystem>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

// Latest version of date on conan is date/2.4.1 -- without hh_mm_ss...
namespace date {
	namespace detail {
		struct undocumented {
			explicit undocumented() = default;
		};
	}  // namespace detail

	template <class Duration>
	class hh_mm_ss {
		using dfs = detail::decimal_format_seconds<
		    typename std::common_type<Duration, std::chrono::seconds>::type>;

		std::chrono::hours h_;
		std::chrono::minutes m_;
		dfs s_;
		bool neg_;

	public:
		static unsigned CONSTDATA fractional_width = dfs::width;
		using precision = typename dfs::precision;

		CONSTCD11 hh_mm_ss() NOEXCEPT : hh_mm_ss(Duration::zero()) {}

		CONSTCD11 explicit hh_mm_ss(Duration d) NOEXCEPT
		    : h_(std::chrono::duration_cast<std::chrono::hours>(
		          detail::abs(d))),
		      m_(std::chrono::duration_cast<std::chrono::minutes>(
		             detail::abs(d)) -
		         h_),
		      s_(detail::abs(d) - h_ - m_),
		      neg_(d < Duration::zero()) {}

		CONSTCD11 std::chrono::hours hours() const NOEXCEPT { return h_; }
		CONSTCD11 std::chrono::minutes minutes() const NOEXCEPT { return m_; }
		CONSTCD11 std::chrono::seconds seconds() const NOEXCEPT {
			return s_.seconds();
		}
		CONSTCD14 std::chrono::seconds& seconds(detail::undocumented) NOEXCEPT {
			return s_.seconds();
		}
		CONSTCD11 precision subseconds() const NOEXCEPT {
			return s_.subseconds();
		}
		CONSTCD11 bool is_negative() const NOEXCEPT { return neg_; }

		CONSTCD11 explicit operator precision() const NOEXCEPT {
			return to_duration();
		}
		CONSTCD11 precision to_duration() const NOEXCEPT {
			return (s_.to_duration() + m_ + h_) * (1 - 2 * neg_);
		}

		CONSTCD11 bool in_conventional_range() const NOEXCEPT {
			return !neg_ && h_ < days{1} && m_ < std::chrono::hours{1} &&
			       s_.in_conventional_range();
		}

	private:
		template <class charT, class traits>
		friend std::basic_ostream<charT, traits>& operator<<(
		    std::basic_ostream<charT, traits>& os,
		    hh_mm_ss const& tod) {
			if (tod.is_negative()) os << '-';
			if (tod.h_ < std::chrono::hours{10}) os << '0';
			os << tod.h_.count() << ':';
			if (tod.m_ < std::chrono::minutes{10}) os << '0';
			os << tod.m_.count() << ':' << tod.s_;
			return os;
		}

		template <class CharT, class Traits, class Duration2>
		friend std::basic_ostream<CharT, Traits>& date::to_stream(
		    std::basic_ostream<CharT, Traits>& os,
		    const CharT* fmt,
		    const fields<Duration2>& fds,
		    const std::string* abbrev,
		    const std::chrono::seconds* offset_sec);

		template <class CharT, class Traits, class Duration2, class Alloc>
		friend std::basic_istream<CharT, Traits>& date::from_stream(
		    std::basic_istream<CharT, Traits>& is,
		    const CharT* fmt,
		    fields<Duration2>& fds,
		    std::basic_string<CharT, Traits, Alloc>* abbrev,
		    std::chrono::minutes* offset);
	};
}  // namespace date

namespace mgps {
	namespace ch = std::chrono;
	namespace fs = std::filesystem;

	using local_ms = date::local_time<ch::milliseconds>;

	// epoch: start of playlist; tracks progression of movie clips
	struct playback_clock {};
	template <typename Duration>
	using playback_time = ch::time_point<playback_clock, Duration>;
	using playback_ms = playback_time<ch::milliseconds>;

	struct timeline_still_item {
		playback_ms offset;
	};

	struct timeline_item {
		playback_ms offset;
		ch::milliseconds duration;
	};
}  // namespace mgps
