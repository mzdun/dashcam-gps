#include <dlfcn.h>

#include <cstring>
#include <system_error>

#include "mgps/loader/loader.hh"

namespace mgps::loader {
	namespace {
		template <typename PrivateClass>
		void* to_platform(PrivateClass* ptr) {
			return static_cast<void*>(ptr);
		}

		template <typename To, typename Prototype>
		void* cast_fun(Prototype* fun) {
			return *reinterpret_cast<void**>(&fun);
		}

		struct fcloser {
			void operator()(FILE* fp) { fclose(fp); }
		};
	}  // namespace

	char const* loader::last_error() { return dlerror(); }

	// adapted from https://stackoverflow.com/a/54764051/6374236
	std::filesystem::path loader::current_library_path(std::error_code& ec) {
		Dl_info di;
		if (!dladdr(cast_fun<void*>(&current_library_path), &di)) {
			ec = std::make_error_code(std::errc::state_not_recoverable);
			return {};
		}

		if (strrchr(di.dli_fname, '/')) return di.dli_fname;

		std::unique_ptr<FILE, fcloser> fp{fopen("/proc/self/maps", "r")};
		if (!fp) {
			ec = std::make_error_code(std::errc::no_such_file_or_directory);
			return {};
		}

		constexpr size_t BUFFER_SIZE = 1024;
		char buffer[BUFFER_SIZE] = "";
		char path[BUFFER_SIZE] = "";

		while (fgets(buffer, BUFFER_SIZE, fp.get())) {
			if (sscanf(buffer, "%*s-%*s %*s %*s %*s %*s %s", path) == 1) {
				auto candidate = std::filesystem::path{path};
				if (candidate.filename() == di.dli_fname) {
					return candidate;
				}
			}
		}

		ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return {};
	}

	loader::handle* loader::load(char const* libname) noexcept {
		auto so = dlopen(libname, RTLD_LAZY);
#if defined(ANDROID) || defined(__ANDROID__)
		if (so) {
			(void)dlerror();  // random "JNI_OnLoad missing" error
		}
#endif
		return reinterpret_cast<handle*>(so);
	}

	void loader::unload(handle* so) noexcept { dlclose(to_platform(so)); }

	void* loader::resolve(char const* function) {
		return dlsym(to_platform(lib_.get()), function);
	}

}  // namespace mgps::loader
