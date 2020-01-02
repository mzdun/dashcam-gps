#include <iostream>
#include "mgps-70mai/loader.hh"
#include "svg.hh"

int main(int argc, char** argv) {
	using namespace mgps;
	using namespace library;

	if (argc < 2) {
		std::cerr << "playground <dir>\n";
		return 1;
	}

	using namespace std::literals;
	constexpr auto gap = 10min;

	mai::loader loader{};
	loader.add_directory(argv[1]);
	auto lib = loader.build(gap);

	svg::html_trace(lib);
}
