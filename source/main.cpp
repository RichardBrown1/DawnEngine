#pragma once
#include <iostream>
#include "../include/DawnEngine.hpp"

int main()
{
	try {
		DawnEngine wgpuEngine = DawnEngine();
		wgpuEngine.run();
		wgpuEngine.destroy();
	}
	catch (std::exception& err) {
		std::cout << "std::Exception: " << err.what() << std::endl;
	}
	catch (...) {
		std::cout << "unknown error" << std::endl;
	}

	return 0;
}
