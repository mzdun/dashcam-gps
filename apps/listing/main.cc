#include <fstream>
#include <iostream>
#include <mgps/library.hh>
#include <mgps/trip.hh>

#include "listing.hh"

int main(int argc, char** argv) {
	using namespace mgps;

	if (argc < 3) {
		std::cerr << "listing <output> <dir> [<dir>...]\n";
		return 1;
	}

	using namespace std::literals;

	library lib{};

	{
		std::error_code ec;
		lib.lookup_plugins(ec);
		if (ec) {
			std::cerr << "Error while loading plugins: " << ec.message().c_str()
			          << '\n';
			return 1;
		}
	}

	lib.before_update();
	for (int argn = 2; argn < argc; ++argn) {
		lib.add_directory(argv[argn]);
	}
	lib.after_update();

	if (argv[1] != "-"sv) {
		std::ofstream out{argv[1]};
		if (out.is_open()) {
			listing(out, lib);
			return 0;
		}
	}

	listing(std::cout, lib);
}
