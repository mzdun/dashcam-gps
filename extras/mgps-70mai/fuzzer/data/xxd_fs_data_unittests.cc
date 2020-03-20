#include "xxd_fs_data_unittests.hh"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace mgps::plugin::isom::fuzzer {
	namespace {
		struct file_data {
			std::string_view filename;
			unsigned char const* data;
			std::size_t length;
		};

		using namespace std::literals;

#include "xxd.h"
		constexpr file_data files[] = {
		    {
		        "NO20191130-120156-000121.MP4"sv,
		        box_mp4,
		        box_mp4_len,
		    },
		    {
		        "NO20191130-120256-000122.MP4"sv,
		        id_000000_src_000000_op_arith8_pos_448_val__34_mp4,
		        id_000000_src_000000_op_arith8_pos_448_val__34_mp4_len,
		    },
		    {
		        "NO20191130-120356-000123.MP4"sv,
		        id_000001_src_000000_op_havoc_rep_64_mp4,
		        id_000001_src_000000_op_havoc_rep_64_mp4_len,
		    },
		    {
		        "NO20191130-120456-000124.MP4"sv,
		        id_000002_src_000000_op_havoc_rep_128_mp4,
		        id_000002_src_000000_op_havoc_rep_128_mp4_len,
		    },
		    {
		        "NO20191130-120556-000125.MP4"sv,
		        id_000003_src_000000_op_havoc_rep_64_mp4,
		        id_000003_src_000000_op_havoc_rep_64_mp4_len,
		    },
		    {
		        "NO20191130-120656-000126.MP4"sv,
		        id_000004_src_000015_op_havoc_rep_32_mp4,
		        id_000004_src_000015_op_havoc_rep_32_mp4_len,
		    },
		    {
		        "NO20191130-120756-000127.MP4"sv,
		        id_000005_src_000019_op_int32_pos_695_val__256_mp4,
		        id_000005_src_000019_op_int32_pos_695_val__256_mp4_len,
		    },
		    {
		        "XA20191130-120856-000128.MP4"sv,
		        id_000006_src_000022_op_arith8_pos_448_val__34_mp4,
		        id_000006_src_000022_op_arith8_pos_448_val__34_mp4_len,
		    },
		    {
		        "BY20191130-120956-000129.MP4"sv,
		        id_000007_src_000064_op_havoc_rep_64_mp4,
		        id_000007_src_000064_op_havoc_rep_64_mp4_len,
		    },
		    {
		        "EV20191130-121056-000130.MP4"sv,
		        id_000008_src_000065_op_havoc_rep_32_mp4,
		        id_000008_src_000065_op_havoc_rep_32_mp4_len,
		    },
		    {
		        "EV20191130-121156-000131.MP4"sv,
		        id_000009_src_000083_op_havoc_rep_64_mp4,
		        id_000009_src_000083_op_havoc_rep_64_mp4_len,
		    },
		    {
		        "EV20191130-121256-000132.MP4"sv,
		        id_000010_src_000105_op_arith8_pos_448_val__34_mp4,
		        id_000010_src_000105_op_arith8_pos_448_val__34_mp4_len,
		    },
		    {
		        "EV20191130-121356-000133.MP4"sv,
		        id_000011_src_000106_op_arith8_pos_448_val__34_mp4,
		        id_000011_src_000106_op_arith8_pos_448_val__34_mp4_len,
		    },
		    {
		        "EV20191130-121456-000134.MP4"sv,
		        id_000012_src_000119_op_arith8_pos_416_val__34_mp4,
		        id_000012_src_000119_op_arith8_pos_416_val__34_mp4_len,
		    },
		    {
		        "PA20191130-121556-000135.MP4"sv,
		        id_000013_src_000124_op_arith8_pos_448_val__34_mp4,
		        id_000013_src_000124_op_arith8_pos_448_val__34_mp4_len,
		    },
		    {
		        "PA20191130-121656-000136.MP4"sv,
		        id_000014_src_000146_op_arith8_pos_448_val__34_mp4,
		        id_000014_src_000146_op_arith8_pos_448_val__34_mp4_len,
		    },
		    {
		        "PA20191130-121756-000137.MP4"sv,
		        id_000015_src_000067_op_arith8_pos_448_val__34_mp4,
		        id_000015_src_000067_op_arith8_pos_448_val__34_mp4_len,
		    },
		    {
		        "PA20191130-121856-000138.MP4"sv,
		        id_000016_src_000086_op_arith8_pos_448_val__34_mp4,
		        id_000016_src_000086_op_arith8_pos_448_val__34_mp4_len,
		    },
		    {
		        "NO20191130-121956-000139.MP4"sv,
		        id_000017_src_000162_op_havoc_rep_16_mp4,
		        id_000017_src_000162_op_havoc_rep_16_mp4_len,
		    },
		    {
		        "NO20191130-122056-000140.MP4"sv,
		        id_000018_src_000162_op_havoc_rep_2_mp4,
		        id_000018_src_000162_op_havoc_rep_2_mp4_len,
		    },
		    {
		        "NO20191130-122156-000141.MP4"sv,
		        id_000019_src_000188_op_arith8_pos_448_val__34_mp4,
		        id_000019_src_000188_op_arith8_pos_448_val__34_mp4_len,
		    },
		    {
		        "EV20191130-122256-000142.MP4"sv,
		        id_000020_src_000188_op_havoc_rep_8_mp4,
		        id_000020_src_000188_op_havoc_rep_8_mp4_len,
		    },
		    {
		        "EV20191130-122356-000143.MP4"sv,
		        id_000021_src_000188_op_havoc_rep_16_mp4,
		        id_000021_src_000188_op_havoc_rep_16_mp4_len,
		    },
		    {
		        "EV20191130-122456-000144.MP4"sv,
		        id_000022_src_000189_op_havoc_rep_4_mp4,
		        id_000022_src_000189_op_havoc_rep_4_mp4_len,
		    },
		    {
		        "PA20191130-122556-000145.MP4"sv,
		        id_000023_src_000072_op_havoc_rep_64_mp4,
		        id_000023_src_000072_op_havoc_rep_64_mp4_len,
		    },
		    {
		        "PA20191130-122656-000146.MP4"sv,
		        id_000024_src_000161_op_arith8_pos_448_val__34_mp4,
		        id_000024_src_000161_op_arith8_pos_448_val__34_mp4_len,
		    },
		    {
		        "NO20191130-122756-000147.MP4"sv,
		        id_000025_src_000161_op_havoc_rep_16_mp4,
		        id_000025_src_000161_op_havoc_rep_16_mp4_len,
		    },
		    {
		        "NO20191130-122856-000148.MP4"sv,
		        id_000026_src_000164_op_havoc_rep_32_mp4,
		        id_000026_src_000164_op_havoc_rep_32_mp4_len,
		    },
		    {
		        "NO20191130-122956-000149.MP4"sv,
		        id_000027_src_000181_op_arith8_pos_1618_val__15_mp4,
		        id_000027_src_000181_op_arith8_pos_1618_val__15_mp4_len,
		    },
		    {
		        "XX20191130-123056-000150.MP4"sv,
		        id_000028_src_000188_op_havoc_rep_4_mp4,
		        id_000028_src_000188_op_havoc_rep_4_mp4_len,
		    },
		    {
		        "YY20191130-123156-000151.MP4"sv,
		        id_000029_src_000190_op_havoc_rep_16_mp4,
		        id_000029_src_000190_op_havoc_rep_16_mp4_len,
		    },
		    {
		        "ZZ20191130-123256-000152.MP4"sv,
		        id_000030_src_000191_op_arith8_pos_448_val__34_mp4,
		        id_000030_src_000191_op_arith8_pos_448_val__34_mp4_len,
		    },
		    {
		        "AA20191130-123356-000153.MP4"sv,
		        id_000031_src_000191_op_havoc_rep_32_mp4,
		        id_000031_src_000191_op_havoc_rep_32_mp4_len,
		    },
		    {
		        "BB20191130-123456-000154.MP4"sv,
		        This_is_not_an_ISOM_file,
		        This_is_not_an_ISOM_file_len,
		    },
		    {
		        "NO20191130-123556-000155.MP4"sv,
		        practicaly_empty,
		        practicaly_empty_len,
		    },
		    {
		        "NO20191130-123656-000156.MP4"sv,
		        nearly_empty,
		        nearly_empty_len,
		    },
		};

		class storage : public isom::storage {
			file_data const* bits_;
			uint64_t position_{0};

		public:
			storage(file_data const* bits) : bits_{bits} {}

			uint64_t offset() const noexcept override { return 0u; }

			bool eof() const noexcept override {
				return position_ == bits_->length;
			}

			uint64_t load(void* buffer, uint64_t length) override {
				auto limited = bits_->length - position_;

				if (limited > length) limited = length;
				std::memcpy(buffer, bits_->data + position_, limited);
				position_ += limited;
				return limited;
			}

			uint64_t tell() override { return position_; }

			uint64_t seek(uint64_t offset) override {
				if (offset > bits_->length) {
					offset = bits_->length;
				}

				position_ = offset;
				return position_;
			}

			uint64_t seek_end() override {
				position_ = bits_->length;
				return position_;
			}

			bool valid() const noexcept override { return !!bits_; }
		};

		std::unique_ptr<isom::storage> invalid() {
			return std::make_unique<storage>(nullptr);
		}
	}  // namespace

	std::unique_ptr<isom::storage> fs_data::open(char const* filename) {
		if (!filename) return invalid();
		auto it = std::find_if(
		    std::begin(files), std::end(files),
		    [filename](auto const& file) { return file.filename == filename; });
		if (it == std::end(files)) return invalid();

		return std::make_unique<storage>(it);
	}

}  // namespace mgps::plugin::isom::fuzzer
