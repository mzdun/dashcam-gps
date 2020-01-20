#pragma once

#if !defined(ANDROID) && defined(__ANDROID__)
#define ANDROID 1
#endif

#if defined(ANDROID)
#include <android/log.h>
#else
#include <cstdio>
#endif
#include <cstring>
#include <sstream>

#define MAKE_LOG(string)                                              \
                                                                      \
	namespace {                                                       \
		struct LOG_TAG__ {                                            \
			static char const* tag_name() noexcept { return string; } \
		};                                                            \
		using Logger = android::logger<LOG_TAG__>;                    \
	}

enum class LogLevel {
	VERBOSE = 2,
	DEBUG,
	INFO,
	WARN,
	ERROR,
	FATAL,
};

#define LOG_LVL(LVL) ::LogLevel::LVL

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_FILE)
#define LOG(LVL) \
	Logger { LOG_LVL(LVL) }
#else
#define LOG(LVL) \
	Logger { LOG_LVL(LVL), __FILE__, __LINE__ }
#endif

namespace android {
#if __has_builtin(__builtin_FILE)
	struct source_location {
		static constexpr source_location current(
		    const char* file = __builtin_FILE(),
		    int line = __builtin_LINE()) noexcept {
			source_location loc;
			loc.file_ = file;
			loc.line_ = line;
			return loc;
		}

		constexpr source_location() noexcept : file_("unknown"), line_(0) {}

		constexpr const char* file_name() const noexcept { return file_; }
		constexpr int line() const noexcept { return line_; }

	private:
		const char* file_;
		int line_;
	};
#endif  // has __builtin_FILE

	template <class TagName>
	class logger {
	public:
#if __has_builtin(__builtin_FILE)
		logger(LogLevel level,
		       source_location const location = source_location::current())
		    : level_{level}, location_{location} {}
#else
		logger(LogLevel level, const char* file, uint_least32_t line)
		    : level_{level}, file_{file}, line_{line} {}
#endif

		~logger() {
			if (dirty_) {
#if defined(ANDROID)
				__android_log_print(static_cast<int>(level_),
				                    TagName::tag_name(), "[%s:%u] > %s",
#if __has_builtin(__builtin_FILE)
				                    shorten(location_.file_name()),
				                    location_.line(),
#else
				                    shorten(file_), line_,
#endif
				                    o_.str().c_str());
#else  // ANDROID
				auto const level = [](LogLevel lvl) {
					switch (lvl) {
						case LogLevel::VERBOSE:
							return "V";
						case LogLevel::DEBUG:
							return "D";
						case LogLevel::INFO:
							return "I";
						case LogLevel::WARN:
							return "W";
						case LogLevel::ERROR:
							return "E";
						case LogLevel::FATAL:
							return "F";
						default:
							return "?";
					}
				}(level_);
				std::printf("%s/%s [%s:%u] > %s\n", level, TagName::tag_name(),
#if __has_builtin(__builtin_FILE)
				            location_.file_name(), location_.line(),
#else
				            file_, line_,
#endif
				            o_.str().c_str());
#endif  // ANDROID
				if (level_ == LogLevel::FATAL) {
#if __has_builtin(__builtin_trap)
					__builtin_trap();
#else
					std::abort();
#endif
				}
			}
		}

		template <typename Arg>
		logger& operator<<(Arg&& arg) {
			o_ << std::forward<Arg>(arg);
			dirty_ = true;
			return *this;
		}

	private:
		static const char* shorten(const char* filename) {
			auto slash = std::strrchr(filename, '/');
			if (!slash) slash = std::strrchr(filename, '\\');
			if (!slash) return filename;
			return slash + 1;
		}
		std::ostringstream o_{};
		bool dirty_{false};
		LogLevel const level_;
#if __has_builtin(__builtin_FILE)
		source_location const location_;
#else
		const char* file_;
		uint_least32_t line_;
#endif
	};
}  // namespace android
