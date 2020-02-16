#include <mgps/loader/parser/elf_parser.hh>
#include <mgps/loader/parser/file_ptr.hh>
#include <vector>

namespace mgps::loader {
	struct shdr_32 {
		std::uint32_t sh_name;
		std::uint32_t sh_type;
		std::uint32_t sh_flags;
		std::uint32_t sh_addr;
		std::uint32_t sh_offset;
		std::uint32_t sh_size;
		std::uint32_t sh_link;
		std::uint32_t sh_info;
		std::uint32_t sh_addralign;
		std::uint32_t sh_entsize;
	};

	struct shdr_64 {
		std::uint32_t sh_name;
		std::uint32_t sh_type;
		std::uint64_t sh_flags;
		std::uint64_t sh_addr;
		std::uint64_t sh_offset;
		std::uint64_t sh_size;
		std::uint32_t sh_link;
		std::uint32_t sh_info;
		std::uint64_t sh_addralign;
		std::uint64_t sh_entsize;
	};

	static_assert(sizeof(shdr_64) ==
	                  (sizeof(std::uint32_t) + sizeof(std::uint32_t) +
	                   sizeof(std::uint64_t) + sizeof(std::uint64_t) +
	                   sizeof(std::uint64_t) + sizeof(std::uint64_t) +
	                   sizeof(std::uint32_t) + sizeof(std::uint32_t) +
	                   sizeof(std::uint64_t) + sizeof(std::uint64_t)),
	              "there is some padding in shdr_32, the code needs to be "
	              "re-written for this platfrom");

	struct header_32 {
		std::uint16_t e_type;
		std::uint16_t e_machine;
		std::uint32_t e_version;
		std::uint32_t e_entry;
		std::uint32_t e_phoff;
		std::uint32_t e_shoff;
		std::uint32_t e_flags;
		std::uint16_t e_ehsize;
		std::uint16_t e_phentsize;
		std::uint16_t e_phnum;
		std::uint16_t e_shentsize;
		std::uint16_t e_shnum;
		std::uint16_t e_shstrndx;

		using shdr = shdr_32;
	};

	static_assert(sizeof(header_32) ==
	                  (sizeof(std::uint16_t) + sizeof(std::uint16_t) +
	                   sizeof(std::uint32_t) + sizeof(std::uint32_t) +
	                   sizeof(std::uint32_t) + sizeof(std::uint32_t) +
	                   sizeof(std::uint32_t) + sizeof(std::uint16_t) +
	                   sizeof(std::uint16_t) + sizeof(std::uint16_t) +
	                   sizeof(std::uint16_t) + sizeof(std::uint16_t) +
	                   sizeof(std::uint16_t)),
	              "there is some padding in header_32, the code needs to be "
	              "re-written for this platfrom");

	struct header_64 {
		std::uint16_t e_type;
		std::uint16_t e_machine;
		std::uint32_t e_version;
		std::uint64_t e_entry;
		std::uint64_t e_phoff;
		std::uint64_t e_shoff;
		std::uint32_t e_flags;
		std::uint16_t e_ehsize;
		std::uint16_t e_phentsize;
		std::uint16_t e_phnum;
		std::uint16_t e_shentsize;
		std::uint16_t e_shnum;
		std::uint16_t e_shstrndx;

		using shdr = shdr_64;
	};

	static_assert(sizeof(header_64) ==
	                  (sizeof(std::uint16_t) + sizeof(std::uint16_t) +
	                   sizeof(std::uint32_t) + sizeof(std::uint64_t) +
	                   sizeof(std::uint64_t) + sizeof(std::uint64_t) +
	                   sizeof(std::uint32_t) + sizeof(std::uint16_t) +
	                   sizeof(std::uint16_t) + sizeof(std::uint16_t) +
	                   sizeof(std::uint16_t) + sizeof(std::uint16_t) +
	                   sizeof(std::uint16_t)),
	              "there is some padding in header_64, the code needs to be "
	              "re-written for this platfrom");

	enum : unsigned char { CLASSNONE, CLASS32, CLASS64, VER_CURRENT = 1 };

	template <typename HeaderType>
	lib_has elf_parser_info_bin(file_ptr& file, plugin_info& out) {
		using SectionHeader = typename HeaderType::shdr;

		HeaderType hdr;
		if (!file.read(hdr)) {
			return lib_has::incorrect_binary_format;
		}

		if (hdr.e_shentsize > sizeof(SectionHeader)) {
			return lib_has::incorrect_binary_format;
		}

		if (hdr.e_shnum <= hdr.e_shstrndx) {
			return lib_has::incorrect_binary_format;
		}

		auto const sh_offset = static_cast<uint64_t>(hdr.e_shoff);
		if (file.seek(sh_offset) != sh_offset) {
			return lib_has::incorrect_binary_format;
		}

		std::vector<unsigned char> sh_buffer(
		    static_cast<size_t>(hdr.e_shentsize) *
		    static_cast<size_t>(hdr.e_shnum));
		if (!file.read(sh_buffer.data(), sh_buffer.size())) {
			return lib_has::incorrect_binary_format;
		}

		std::vector<char> strings;
		{
			auto strsec = reinterpret_cast<SectionHeader const*>(
			    sh_buffer.data() + static_cast<size_t>(hdr.e_shentsize) *
			                           static_cast<size_t>(hdr.e_shstrndx));

			auto const str_offset = static_cast<uint64_t>(strsec->sh_offset);
			if (file.seek(str_offset) != str_offset) {
				return lib_has::incorrect_binary_format;
			};

			auto const str_size = static_cast<size_t>(strsec->sh_size);
			strings.resize(str_size);
			if (!file.read(strings.data(), strings.size())) {
				return lib_has::incorrect_binary_format;
			}
			if (strings.empty() || strings.back() != 0) strings.push_back(0);
		}

		using namespace std::literals;
		static constexpr auto _mgpsnfo = ".mgpsnfo"sv;

		auto shdrs = sh_buffer.data();
		for (decltype(hdr.e_shnum) ndx = 0; ndx < hdr.e_shnum; ++ndx) {
			auto shdr = reinterpret_cast<SectionHeader const*>(shdrs);
			shdrs += static_cast<size_t>(hdr.e_shentsize);

			if (ndx == hdr.e_shstrndx) continue;
			if (shdr->sh_name >= strings.size()) continue;
			auto const name = strings.data() + shdr->sh_name;
			if (name != _mgpsnfo) continue;

			auto const sec_offset = static_cast<uint64_t>(shdr->sh_offset);
			if (file.seek(sec_offset) != sec_offset) {
				return lib_has::incorrect_binary_format;
			}

			auto const size = static_cast<uint64_t>(shdr->sh_size);
			std::vector<unsigned char> buffer(size);
			if (!file.read(buffer.data(), buffer.size())) {
				return lib_has::incorrect_binary_format;
			}

			return info_from_plugin_info({buffer.data(), buffer.size()}, out);
		}

		return lib_has::no_needed_segments;
	}

	lib_has elf_parser_info(std::string const& path,
	                        size_t ptr_size,
	                        plugin_info& out) {
		auto file = file_ptr{std::fopen(path.c_str(), "rb")};
		if (!file) return lib_has::existance_issues;

		unsigned char magic[16];
		if (!file.read(magic)) {
			return lib_has::incorrect_binary_format;
		}

#define TEST(NDX, UCHAR)                         \
	if (magic[NDX] != (UCHAR)) {                 \
		return lib_has::incorrect_binary_format; \
	}
		TEST(0, 0x7f)
		TEST(1, 'E')
		TEST(2, 'L')
		TEST(3, 'F')
		TEST(4, ptr_size == 4 ? CLASS32 : (ptr_size == 8 ? CLASS64 : CLASSNONE))
		TEST(6, VER_CURRENT)
#undef TEST

		auto bits = magic[4];
		if (bits != CLASS32 && bits != CLASS64) {
			return lib_has::incorrect_binary_format;
		}

		if (bits == CLASS32) {
			return elf_parser_info_bin<header_32>(file, out);
		}
		return elf_parser_info_bin<header_64>(file, out);
	}
}  // namespace mgps::loader
