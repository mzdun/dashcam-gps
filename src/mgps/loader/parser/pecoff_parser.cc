#include <mgps/loader/parser/file_ptr.hh>
#include <mgps/loader/parser/pecoff_parser.hh>
#include <vector>

namespace mgps::loader {
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
			if (!file.read(buffer.data(), buffer.size())) {
				return lib_has::incorrect_binary_format;
			}

			return info_from_plugin_info({buffer.data(), buffer.size()}, out);
		}

		return lib_has::no_needed_segments;
	}
}  // namespace mgps::loader
