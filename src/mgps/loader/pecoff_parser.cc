#include <mgps/loader/pecoff_parser.hh>

namespace mgps::loader {
	struct fcloser {
		void operator()(FILE* f) { std::fclose(f); }
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

	enum : std::uint32_t {
		OFFSET_POS = 0x3Cu,
		PE_SIGNATURE = 0x00'00'45'50u,  // PE\0\0
	};
	enum : std::uint16_t { IMAGE_FILE_32BIT_MACHINE = 0x0100 };
	enum : std::uint64_t { _mgpsnfo = 0x6F666E7370676D2Eull };

	struct signature_and_header {
		std::uint32_t pe_signature;
		std::uint16_t machine;
		std::uint16_t section_count;
		std::uint32_t timestamp;
		std::uint32_t symbol_tbl;
		std::uint32_t symbol_count;
		std::uint16_t optional_header_size;
		std::uint16_t characteristics;
	};

	static_assert(sizeof(signature_and_header) ==
	                  (sizeof(std::uint32_t) + sizeof(std::uint16_t) +
	                   sizeof(std::uint16_t) + sizeof(std::uint32_t) +
	                   sizeof(std::uint32_t) + sizeof(std::uint32_t) +
	                   sizeof(std::uint16_t) + sizeof(std::uint16_t)),
	              "there is some padding in signature_and_header, the code "
	              "needs to be re-written for this platfrom");

	struct pe_section {
		std::uint64_t name;
		std::uint32_t virtual_size;
		std::uint32_t virtual_address;
		std::uint32_t size_of_raw_data;
		std::uint32_t pointer_to_raw_data;
		std::uint32_t pointer_to_relocations;
		std::uint32_t pointer_to_linenumbers;
		std::uint16_t number_of_relocations;
		std::uint16_t number_of_linenumbers;
		std::uint32_t characteristics;
	};

	static_assert(sizeof(pe_section) ==
	                  (sizeof(std::uint64_t) + sizeof(std::uint32_t) +
	                   sizeof(std::uint32_t) + sizeof(std::uint32_t) +
	                   sizeof(std::uint32_t) + sizeof(std::uint32_t) +
	                   sizeof(std::uint32_t) + sizeof(std::uint16_t) +
	                   sizeof(std::uint16_t) + sizeof(std::uint32_t)),
	              "there is some padding in pe_section, the code needs to be "
	              "re-written for this platfrom");

	lib_has pecoff_parser_info(std::string const& path,
	                           size_t ptr_size,
	                           plugin_info& out) {
		auto file = file_ptr{std::fopen(path.c_str(), "rb")};
		if (!file) return lib_has::existance_issues;

		char dos_stub[OFFSET_POS + sizeof(std::uint32_t)];
		if (!file.read(dos_stub)) {
			return lib_has::incorrect_binary_format;
		}

		if (dos_stub[0] != 'M' || dos_stub[1] != 'Z') {
			return lib_has::incorrect_binary_format;
		}

		auto const signature_offset =
		    *reinterpret_cast<std::uint32_t*>(dos_stub + OFFSET_POS);
		if (file.seek(signature_offset) != signature_offset) {
			return lib_has::incorrect_binary_format;
		}

		signature_and_header head;

		if (!file.read(head) || head.pe_signature != PE_SIGNATURE) {
			return lib_has::incorrect_binary_format;
		}

		auto const is_32bit =
		    (head.characteristics & IMAGE_FILE_32BIT_MACHINE) ==
		    IMAGE_FILE_32BIT_MACHINE;
		auto const expects_32bit = ptr_size == 4;
		if (is_32bit != expects_32bit) {
			return lib_has::unexpected_architecture;
		}

		std::uint64_t const segment_table_offset =
		    file.tell() + head.optional_header_size;
		if (file.seek(segment_table_offset) != segment_table_offset) {
			return lib_has::incorrect_binary_format;
		}

		for (decltype(head.section_count) sec_ndx = 0;
		     sec_ndx < head.section_count; ++sec_ndx) {
			pe_section sec;
			if (!file.read(sec)) {
				return lib_has::incorrect_binary_format;
			}

			if (sec.name != _mgpsnfo) continue;

			if (file.seek(sec.pointer_to_raw_data) != sec.pointer_to_raw_data) {
				return lib_has::incorrect_binary_format;
			}

			auto const size = std::min(sec.size_of_raw_data, sec.virtual_size);
			std::vector<unsigned char> buffer(size);
			if (!file.read(buffer.data(), size)) {
				return lib_has::incorrect_binary_format;
			}

			return info_from_plugin_info({buffer.data(), size}, out);
		}

		return lib_has::no_needed_segments;
	}
}  // namespace mgps::loader
