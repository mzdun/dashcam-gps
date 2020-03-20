#pragma once

#include <mgps/plugin/isom.hh>

namespace mgps::plugin::isom::fuzzer {
	struct fs_data : public isom::fs_data {
		std::unique_ptr<isom::storage> open(char const* filename) override;
	};
}  // namespace mgps::plugin::isom::fuzzer
