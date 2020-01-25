#pragma once
#include <filesystem>
#include <memory>

namespace mgps::plugins::host {
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

		static char const* last_error();
		static std::filesystem::path current_library_path(std::error_code& ec);

	private:
		class handle;

		static handle* load(char const*) noexcept;
		static void unload(handle*) noexcept;

		void* resolve(char const*);

		struct unloader {
			void operator()(handle* lib) { unload(lib); }
		};

		std::unique_ptr<handle, unloader> lib_{nullptr};
	};
}  // namespace mgps::host
