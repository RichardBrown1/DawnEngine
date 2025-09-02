#include <string>
#include <iostream>
#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "engine/engine.hpp"

int main()
{
	absl::SetStderrThreshold(LOG_LEVEL);

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
