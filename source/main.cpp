#pragma once
#include <iostream>
#include "../include/engine.hpp"

int main()
{
	try {
		Engine dawnEngine = Engine();
		dawnEngine.run();
	}
	catch (std::exception& err) {
		std::cout << "std::Exception: " << err.what() << std::endl;
	}
	catch (...) {
		std::cout << "unknown error" << std::endl;
	}

	return 0;
}
