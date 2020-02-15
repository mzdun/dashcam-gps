#pragma once
#include <vector>
#include <iostream>

namespace mgps {
	class library;
	struct trip;
}

namespace mgps::svg {
	void html_trace(std::ostream&, library const&, std::vector<trip> const&);
}
