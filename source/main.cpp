#pragma once
#include <string>
#include <iostream>
#pragma once
#include "absl/log/log.h"
#include "engine/engine.hpp"

int main()
{
	try {
		Engine engine = Engine();
		engine.run();
	}
	catch (std::exception& err) {
		LOG(FATAL) << err.what();
	}
	catch (...) {
		LOG(FATAL) << "unknown error";
	}

	return 0;
}
