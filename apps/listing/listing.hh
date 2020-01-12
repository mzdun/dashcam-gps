#pragma once
#include <iostream>
#include <vector>

namespace mgps {
	enum class page : int;
	class library;

	void listing(std::ostream&, library const&);
}  // namespace mgps
