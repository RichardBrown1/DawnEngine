namespace DawnEngine {

	//Corresponds to bitset textureOptions[i]
	enum TextureOptionsIndex {
		HAS_BASE_COLOR_TEXTURE = 0,
		HAS_METALLIC_ROUGHNESS_TEXTURE = 1,
	};

	//TODO: Fill this out with more Texture Types.
	enum TextureType {
		COLOR = 0,
		NORMAL = 1,
		METALLIC_ROUGHNESS = 2,
	};

};
