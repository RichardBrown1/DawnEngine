//Oversized triangle is better to draw on than 2 small triangles
float4 vs_main(uint vertexIndex : SV_VertexID) : SV_Position
{
    if (vertexIndex == 0)
    {
        return float4(-1.0, 3.0, 0.0, 1.0);
    }
    else if (vertexIndex == 1)
    {
        return float4(-1.0, -1.0, 0.0, 1.0);
    }
    else
    {
        return float4(3.0, -1.0, 0.0, 1.0);
    }
}
