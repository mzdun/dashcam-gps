#pragma once
#include <vector>
#include <iostream>

namespace mgps {
	struct trip;
}

namespace mgps::svg {
	void html_trace(std::ostream&, std::vector<trip> const&);
}
