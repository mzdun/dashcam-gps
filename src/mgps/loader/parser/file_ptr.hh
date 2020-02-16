#pragma once
#include <cstdio>
#include <memory>

namespace mgps::loader {
	struct fcloser {
		void operator()(std::FILE* f) { std::fclose(f); }
	};
	using file_ptr_base = std::unique_ptr<FILE, fcloser>;
	class file_ptr : file_ptr_base {
	public:
		using file_ptr_base::file_ptr_base;
		using file_ptr_base::operator bool;

		template <typename Item, size_t Length>
		bool read(Item (&buffer)[Length]) noexcept {
			constexpr auto bytes = Length * sizeof(Item);
			return std::fread(buffer, 1, bytes, get()) == bytes;
		}

		template <typename Item>
		bool read(Item* buffer, size_t count) noexcept {
			auto const bytes = count * sizeof(Item);
			return std::fread(buffer, 1, bytes, get()) == bytes;
		}

		template <typename Item>
		bool read(Item& buffer) noexcept {
			constexpr auto bytes = sizeof(Item);
			return std::fread(&buffer, 1, bytes, get()) == bytes;
		}

		uint64_t tell() {
			auto const ret = std::ftell(get());
			if (ret < 0)  // error... TODO: throw
				return 0U;
			return static_cast<unsigned long>(ret);
		}

		uint64_t skip(uint64_t count) { return seek(tell() + count); }

		uint64_t seek(uint64_t offset) {
			static constexpr auto max_chunk =
			    static_cast<uint64_t>(std::numeric_limits<long>::max());
			std::fseek(get(), 0, SEEK_SET);
			while (offset) {
				uint64_t chunk = offset;
				if (chunk > max_chunk) chunk = max_chunk;
				std::fseek(get(), static_cast<long>(chunk), SEEK_CUR);
				offset -= chunk;
			}
			return tell();
		}
	};
}  // namespace mgps::loader
