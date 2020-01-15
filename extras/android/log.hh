#include <android/log.h>
#include <sstream>

#define MAKE_LOG(string)                                              \
	\
namespace {                                                           \
		struct LOG_TAG__ {                                            \
			static char const* tag_name() noexcept { return string; } \
		};                                                            \
		using Logger = android::logger<LOG_TAG__>;                    \
	\
}

#if __has_builtin(__builtin_FILE)
#define LOG(LVL) \
	Logger { ANDROID_LOG_##LVL }
#else
#define LOG(LVL) \
	Logger { ANDROID_LOG_##LVL, __FILE__, __LINE__ }
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
		static constexpr source_location current() noexcept { return {}; }

		constexpr source_location() noexcept
		    : file_("unknown"), func_(file), line_(0) {}

		// 14.1.3, source_location field access
		constexpr uint_least32_t line() const noexcept { return line_; }
		constexpr const char* function_name() const noexcept { return func_; }

	private:
		const char* file_;
		uint_least32_t line_;
	};
#endif  // has __builtin_FILE

	template <class TagName>
	class logger {
	public:
#if __has_builtin(__builtin_FILE)
		logger(int priority,
		       source_location const location = source_location::current())
		    : priority_{priority}, location_{location} {}
#else
		logger(int priority, const char* file, uint_least32_t line)
		    : priority_{priority}, file_{file}, line_{line} {}
#endif

		~logger() {
			if (dirty_) {
				__android_log_print(priority_, TagName::tag_name(),
				                    "[%s:%u] > %s",
#if __has_builtin(__builtin_FILE)
				                    location_.file_name(), location_.line(),
#else
				                    file_, line_,
#endif
				                    o_.str().c_str());
			}
		}

		template <typename Arg>
		logger& operator<<(Arg&& arg) {
			o_ << std::forward<Arg>(arg);
			dirty_ = true;
			return *this;
		}

	private:
		std::ostringstream o_{};
		bool dirty_{false};
		int const priority_;
#if __has_builtin(__builtin_FILE)
		source_location const location_;
#else
		const char* file_;
		uint_least32_t line_;
#endif
	};
}
