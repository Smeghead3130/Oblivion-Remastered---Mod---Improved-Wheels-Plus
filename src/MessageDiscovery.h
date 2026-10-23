#pragma once
#include <cstdint>

namespace messagediscovery
{
	// Only widget discovery is throttled. Known widgets are always checked live;
	// input actions force discovery so a newly created dialog cannot consume a wheel action.
	class Gate
	{
	public:
		bool Due(std::uint64_t now, bool immediate)
		{
			if (!immediate && now < next_) return false;
			next_ = now + 100;
			return true;
		}
	private:
		std::uint64_t next_ = 0;
	};
}
