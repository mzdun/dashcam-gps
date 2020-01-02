#include <mgps/cstdio.hh>

namespace mgps::isom::cstdio {
	uint64_t storage::offset() const noexcept { return 0u; }
	bool storage::eof() const noexcept { return std::feof(bits_.get()); }

	uint64_t storage::load(void* buffer, uint64_t length) {
		return std::fread(buffer, 1, length, bits_.get());
	}

	uint64_t storage::tell() {
		auto const ret = std::ftell(bits_.get());
		if (ret < 0)  // error... TODO: throw
			return 0u;
		return static_cast<unsigned long>(ret);
	}

	uint64_t storage::seek(uint64_t offset) {
		constexpr uint64_t max_chunk = std::numeric_limits<long>::max();
		std::fseek(bits_.get(), 0, SEEK_SET);
		while (offset) {
			uint64_t chunk = offset;
			if (chunk > max_chunk) chunk = max_chunk;
			std::fseek(bits_.get(), static_cast<long>(chunk), SEEK_CUR);
			offset -= chunk;
		}
		return tell();
	}

	uint64_t storage::seek_end() {
		std::fseek(bits_.get(), 0, SEEK_END);
		return tell();
	}

	storage open(char const* path, char const* mode) {
		return file{std::fopen(path, mode)};
	}
}  // namespace mgps::isom::cstdio
