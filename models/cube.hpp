#include <vector>

struct Cube {
	std::vector<float> vertexData = {
        -1.0f, -1.0f, -1.0f, 
         1.0f, -1.0f, -1.0f, 
         1.0f,  1.0f, -1.0f, 
        -1.0f,  1.0f, -1.0f, 
        -1.0f, -1.0f,  1.0f, 
         1.0f, -1.0f,  1.0f, 
         1.0f,  1.0f,  1.0f, 
        -1.0f,  1.0f,  1.0f  
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