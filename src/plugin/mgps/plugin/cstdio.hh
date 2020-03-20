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

		uint64_t offset() const noexcept override;
		bool eof() const noexcept override;
		uint64_t load(void* buffer, uint64_t length) override;
		uint64_t tell() override;
		uint64_t seek(uint64_t offset) override;
		uint64_t seek_end() override;
		bool valid() const noexcept override { return !!bits_; }
	};

	storage open(char const* path, char const* mode = "rb");

	struct fs_data : public isom::fs_data {
		std::unique_ptr<isom::storage> open(char const* filename) override;
	};
}  // namespace mgps::isom::cstdio
