#include <iostream>
#include <mgps-70mai/loader.hh>
#include <mgps/drive.hh>
#include <fstream>

#include "svg.hh"

int main(int argc, char** argv) {
	using namespace mgps;

	if (argc < 2) {
		std::cerr << "playground <dir> [<filename>]\n";
		return 1;
	}

	using namespace std::literals;
	constexpr auto gap = 10min;

	mai::loader loader{};
	loader.add_directory(argv[1]);
	auto lib = loader.build(gap);

	{
		std::error_code ec;
		fs::current_path(argv[1], ec);
	}

	if (argc > 2) {
		std::ofstream out{argv[2]};
		if (out.is_open()) {
			svg::html_trace(out, lib);
			std::error_code ec;
			auto path = fs::weakly_canonical(fs::path{argv[2]}, ec);
			if (ec)
				std::cerr << argv[2] << '\n';
			else
				std::cerr << path.string() << '\n';
			return 0;
		}
	}

	svg::html_trace(std::cout, lib);
}
