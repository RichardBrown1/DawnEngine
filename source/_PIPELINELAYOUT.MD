# Pipeline Layout

## Shadow Map Pipeline
TODO: Implement an accumulator or something to support multiple light sources.
Ideally only shadows within the player's viewport will be calculated
- in
    - vbo
    - lights
    - transforms 
- out
    - shadowmap

## Initial Pipeline
- in
    - vbo
    - camera position
    - transforms 
    - instance properties
    - shadow map
- out
	- texture maps 
        - color
        - TODO: Metallic Roughness / others?
	- normal
	- material / textureCoord
    - shadows from camera perspective

## Texture Map Pipeline (for each type of property)
- in
    - masterTextureInfo from <b> Camera Vertex Pipeline </b>
    	- materialId 
		- texcoords
	- (map of material) Accumulator in
- process
    - add texturemap info to accumulator if materialId matches
- out
    - (map of material) Accumulator out


## Light Map Pipeline
- in 
    - lights
    - normal map after it has been processed by <b> Texture Map Pipeline </b>
- out
    - light map


## Ultimate Pipeline
- in
    - texture maps
    - light map
    - shadow map
- out
    - ultimate result

## ToSurface Pipeline
- in
    - ultimate result
- out
    - renderable surface
