#include <mgps/plugins/host/loader.hh>

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#undef min
#undef max

#include <cctype>
#include <cstdio>
#include <system_error>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace mgps::host {
	namespace {
		template <typename PrivateClass>
		HMODULE to_platform(PrivateClass* ptr) {
			return reinterpret_cast<HMODULE>(ptr);
		}
		template <typename PrivateClass>
		PrivateClass* from_platform(HMODULE dll) {
			return reinterpret_cast<PrivateClass*>(dll);
		}

		inline std::error_code from_win32(DWORD ec) {
			return {static_cast<int>(ec), std::system_category()};
		}

		inline std::error_code from_win32() {
			return from_win32(::GetLastError());
		}
	}  // namespace

	char const* loader::last_error() {
		auto const error = GetLastError();
		if (error == ERROR_SUCCESS) return nullptr;
		SetLastError(ERROR_SUCCESS);

		thread_local char buffer[1024];
		auto length = FormatMessageA(
		    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
		    error, 0, buffer, sizeof(buffer) / sizeof(buffer[0]), nullptr);

		if (!length) {
			sprintf_s(buffer, "unknown error: 0x%08x",
			          static_cast<unsigned>(error));
			return buffer;
		}

		while (length &&
		       std::isspace(static_cast<unsigned char>(buffer[length - 1]))) {
			--length;
		}
		buffer[length] = 0;

		return buffer;
	}

	std::filesystem::path loader::current_library_path(std::error_code& ec) {
		wchar_t local_path[MAX_PATH];
		auto mod = reinterpret_cast<HMODULE>(&__ImageBase);
		auto path = local_path;
		auto size = std::size(local_path);

		std::unique_ptr<wchar_t[]> buffer;

		do {
			auto const result =
			    GetModuleFileNameW(mod, path, static_cast<DWORD>(size));
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				static constexpr auto max_size =
				    static_cast<size_t>(std::numeric_limits<DWORD>::max() / 2);
				if (size > max_size) {
					ec = std::make_error_code(std::errc::not_enough_memory);
					return {};
				}

				try {
					buffer = std::make_unique<wchar_t[]>(2 * size);
					path = buffer.get();
					size *= 2;
					continue;
				} catch (std::bad_alloc&) {
					ec = std::make_error_code(std::errc::not_enough_memory);
					return {};
				}
			}

			if (!result) {
				ec = from_win32();
				return {};
			}

			return path;
		} while (true);
	}

	struct error_mode {
		error_mode() : prev{SetErrorMode(SEM_FAILCRITICALERRORS)} {}
		~error_mode() { SetErrorMode(prev); }
		UINT prev{};
	};

	loader::handle* loader::load(char const* libname) noexcept {
		error_mode hide_dlgs{};  // to hide the unsuccessfull loads of .ilk/.pdb
		auto size = MultiByteToWideChar(CP_UTF8, 0, libname, -1, nullptr, 0);
		if (!size) return from_platform<handle>(LoadLibraryA(libname));

		std::unique_ptr<wchar_t[]> buffer{};
		try {
			buffer = std::make_unique<wchar_t[]>(size);
		} catch (std::bad_alloc&) {
			return from_platform<handle>(LoadLibraryA(libname));
		}

		auto actual =
		    MultiByteToWideChar(CP_UTF8, 0, libname, -1, buffer.get(), size);
		if (!actual) return from_platform<handle>(LoadLibraryA(libname));

		return from_platform<handle>(LoadLibraryW(buffer.get()));
	}

	void loader::unload(handle* dll) noexcept { FreeLibrary(to_platform(dll)); }

	void* loader::resolve(char const* function) {
		return GetProcAddress(to_platform(lib_.get()), function);
	}

}  // namespace mgps::host
