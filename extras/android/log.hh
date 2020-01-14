#include <android/log.h>

#define MAKE_LOG(string)                                              \
	\
namespace {                                                           \
		struct LOG_TAG__ {                                            \
			static char const* tag_name() noexcept { return string; } \
		};                                                            \
		android::logger<LOG_TAG__> const Log{};                       \
	\
}

namespace android {
	template <class TagName>
	struct logger {
		template <int PRIORITY>
		struct messages {
			int operator()(const char* fmt, ...) const noexcept {
				va_list ap;
				va_start(ap, fmt);
				auto const result = __android_log_vprint(
				    PRIORITY, TagName::tag_name(), fmt, ap);
				va_end(ap);
				return result;
			}
			int operator()(std::error_code const& ec) const noexcept {
				return (*this)("%s %d: %s", ec.category().name(), ec.value(),
				               ec.message().c_str());
			}
		};

		messages<ANDROID_LOG_VERBOSE> v{};
		messages<ANDROID_LOG_DEBUG> d{};
		messages<ANDROID_LOG_INFO> i{};
		messages<ANDROID_LOG_WARN> w{};
		messages<ANDROID_LOG_ERROR> e{};
		messages<ANDROID_LOG_FATAL> f{};
	};
}
