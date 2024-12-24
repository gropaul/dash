#pragma once

#include <functional>

class AutoCleaner {
public:
	AutoCleaner(
	    const std::function<void()> &_cleaner) : cleaner(_cleaner) {
	}
	~AutoCleaner() {
		cleaner();
	}

private:
	std::function<void()> cleaner;
};
