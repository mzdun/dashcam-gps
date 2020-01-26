#include <fstream>
#include <iostream>
#include <mgps/library.hh>
#include <mgps/trip.hh>

#include "svg.hh"

int main(int argc, char** argv) {
	using namespace mgps;

	if (argc < 2) {
		std::cerr << "html+svg <dir> [<filename>]\n";
		return 1;
	}

	using namespace std::literals;

	library lib{};
	{
		std::error_code ec;
		lib.lookup_plugins(ec);
		if (ec) {
			std::cerr << "Error while loading plugins: "
			          << ec.message().c_str() << '\n';
			return 1;
		}
	}
	lib.before_update();
	lib.add_directory(argv[1]);
	lib.after_update();
	auto section = lib.build(page::everything, library::default_gap);

	{
		std::error_code ec;
		fs::current_path(argv[1], ec);
	}

	if (argc > 2) {
		std::ofstream out{argv[2]};
		if (out.is_open()) {
			svg::html_trace(out, section);
			std::error_code ec;
			auto path = fs::weakly_canonical(fs::path{argv[2]}, ec);
			if (ec)
				std::cerr << argv[2] << '\n';
			else
				std::cerr << path.string() << '\n';
			return 0;
		}
	}

	svg::html_trace(std::cout, section);
}
