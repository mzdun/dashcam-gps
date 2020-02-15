#pragma once
#include <filesystem>
#include <memory>
#include <mgps/export.hh>

namespace mgps::loader {
	class loader {
	public:
		loader() = default;
		~loader() = default;
		loader(loader&&) = default;
		loader& operator=(loader&&) = default;

		explicit loader(char const* libname) : lib_{load(libname)} {}
		explicit operator bool() const noexcept { return !!lib_; }

		template <typename Prototype>
		Prototype* resolve(char const* function) {
			Prototype* fun{};
			*reinterpret_cast<void**>(&fun) = resolve(function);
			return fun;
		}

		static MGPS_EXPORT char const* last_error();
		static MGPS_EXPORT std::filesystem::path current_library_path(std::error_code& ec);

	private:
		class handle;

		static MGPS_EXPORT handle* load(char const*) noexcept;
		static MGPS_EXPORT void unload(handle*) noexcept;

		MGPS_EXPORT void* resolve(char const*);

		struct unloader {
			void operator()(handle* lib) { unload(lib); }
		};

		std::unique_ptr<handle, unloader> lib_{nullptr};
	};
}  // namespace mgps::host
