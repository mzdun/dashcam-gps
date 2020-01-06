#pragma once
#include <vector>
#include <iostream>

namespace mgps {
	struct drive;
}

namespace mgps::svg {
	void html_trace(std::ostream&, std::vector<drive> const&);
}
