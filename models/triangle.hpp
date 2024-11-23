#include <vector>

struct Triangle {
	std::vector<float> vertexData = {
		-0.5, -0.5,
		+0.5, -0.5,
		+0.0, +0.5,
	};
	std::vector<uint16_t> indexData = {
		0, 1, 2
	};
};