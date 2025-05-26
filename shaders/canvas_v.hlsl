#include "canvas.hlsli"

//Oversized triangle is better to draw on than 2 small triangles
CanvasOutput vs_main(uint vertexIndex : SV_VertexID)
{
    CanvasOutput canvasOutput;
    canvasOutput.position.x = float(vertexIndex / 2) * 4.0 - 1.0;
    canvasOutput.position.y = float(vertexIndex % 2) * 4.0 - 1.0;
    canvasOutput.position.z = 0.0;
    canvasOutput.position.w = 1.0;
    
    canvasOutput.texCoord = float(vertexIndex / 2) * 2.0;
    canvasOutput.texCoord.y = 1.0 - float(vertexIndex % 2) * 2.0;
    
    return canvasOutput;
}
