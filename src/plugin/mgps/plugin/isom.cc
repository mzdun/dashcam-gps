#include <mgps/plugin/isom.hh>

namespace mgps::plugin::isom {
	std::chrono::milliseconds duration_type::to_chrono() const noexcept {
		using namespace std::chrono;

		if (timescale == 0 || length == UNKNOWN_DURATION_32 ||
		    length == UNKNOWN_DURATION_64) {
			return milliseconds::max();
		}

		return milliseconds{(length * 1000u) / timescale};
	}

	bool storage::is_isom() {
		seek(0);
		get<std::uint32_t>();
		if (eof()) return false;
		auto const type = get_type();
		if (eof()) return false;
		switch (type) {
			case box_type::ftyp:
			case box_type::moov:
			case box_type::moof:
			case box_type::styp:
			case box_type::sidx:
			/*Adobe specific*/
			case box_type::afra:
			case box_type::abst:
			case box_type::mdat:
			case box_type::free:
			case box_type::skip:
			case box_type::udta:
			case box_type::meta:
			case box_type::void_:
			case box_type::jP:
			case box_type::wide:
				return true;
			default:
				break;
		}
		return false;
	}

	range_storage::range_storage(storage* stg, uint64_t offset, uint64_t size)
	    : stg_{stg}, lower_{offset}, position_{offset}, upper_{size + offset} {}

	uint64_t range_storage::offset() const noexcept {
		return lower_ + stg_->offset();
	}

	bool range_storage::eof() const noexcept {
		return position_ == upper_ || stg_->eof();
	}

	uint64_t range_storage::load(void* buffer, uint64_t length) {
		auto limited = upper_ - position_;

		if (limited > length) limited = length;
		auto const read = stg_->load(buffer, limited);
		position_ += read;
		return read;
	}

	uint64_t range_storage::tell() { return stg_->tell() - lower_; }

	uint64_t range_storage::seek(uint64_t offset) {
		auto limited = lower_ + offset;
		if (limited > upper_) {
			limited = upper_;
		}

		position_ = stg_->seek(limited);
		return position_ - lower_;
	}

	uint64_t range_storage::seek_end() { return seek(upper_ - lower_); }

	bool boxes::has_box(storage& bits) {
		bits.seek(next_box_at_);
		if (bits.eof()) return false;

		uint64_t size = bits.get_u32();
		if (bits.eof()) return false;

		auto const tag = bits.get_type();
		if (bits.eof()) return false;

		if (!size) {
			auto const here = bits.tell();
			auto const there = bits.seek_end();
			size = there - here + 8;  // it will be deducted on return
		} else if (size == 1) {
			size = bits.get_u64() - 8;
		}

		auto const offset = bits.tell();
		next_box_at_ = offset + size - 8;

		box_ = box_info{offset + bits.offset(), size - 8, tag};
		return true;
	}
}  // namespace mgps::isom
