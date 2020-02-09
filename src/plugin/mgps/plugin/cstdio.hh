#pragma once
#include <cstdio>
#include <memory>
#include <mgps/plugin/isom.hh>

namespace mgps::plugin::isom::cstdio {
	struct fcloser {
		void operator()(FILE* f) { std::fclose(f); }
	};
	using file = std::unique_ptr<FILE, fcloser>;

	class storage : public isom::storage {
		file bits_;

	public:
		storage(file bits) : bits_{std::move(bits)} {}
		bool valid() const noexcept { return !!bits_; }

		uint64_t offset() const noexcept override;
		bool eof() const noexcept override;
		uint64_t load(void* buffer, uint64_t length) override;
		uint64_t tell() override;
		uint64_t seek(uint64_t offset) override;
		uint64_t seek_end() override;
	};

	storage open(char const* path, char const* mode = "rb");
}  // namespace mgps::isom::cstdio
