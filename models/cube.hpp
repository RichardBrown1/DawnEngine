#include <vector>

struct Cube {
	std::vector<float> vertexData = {
		-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, // Red
		 1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // Green
		 1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 1.0f, // Blue
		-1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f, // Yellow
		-1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 1.0f, // Magenta
		 1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // Cyan
		 1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f, // White
		-1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 0.0f  // Black
	};
	std::vector<uint16_t> indexData = {
        0, 1, 2, 2, 3, 0, // Back face
		4, 5, 6, 6, 7, 4, // Front face
		0, 4, 7, 7, 3, 0, // Left face
		1, 5, 6, 6, 2, 1, // Right face
		3, 7, 6, 6, 2, 3, // Top face
		0, 4, 5, 5, 1, 0  // Bottom face
	};
};