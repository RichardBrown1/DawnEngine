//Oversized triangle is better to draw on than 2 small triangles
float4 vs_main(uint vertexIndex : SV_VertexID) : POSITION0
{
    if (vertexIndex == 0)
    {
        return float4(-1, 3, 0, 1);
    }
    else if (vertexIndex == 1)
    {
        return float4(-1, -1, 0, 1);
    }
    else
    {
        return float4(3, 3, 0, 1);
    }
}
