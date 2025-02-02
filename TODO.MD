# TODO
## Multipass Rendering Conversion
- Reduce the current shader 
    - Reduce the bindgroups into 1
- Vertex Shader has to output a normal map
- Vertex Shader has to output a world position map
- Where do I get the color map from?
- Add a second shader that can reconstruct using the depth map

## Thoughts
- Convert to F16 
- 4 Binding Groups regrouped to order eg static / scene, infrequent, frequent, per-frame
    - Investigate Bind Order
- Should static and moving object transforms be put together or separate?
    - Bind Visibility 
    - Bind Type
- Preprocessor Macro Binding and Binding Group or maybe just create a central registry to manage it neatly?