// Helper function to compute light direction from XYZ Euler angles (radians) in a left-handed system
float3 ComputeLightDirection(float3 eulerRadians)
{
    const float x = eulerRadians.x; // Pitch (X-axis)
    const float y = eulerRadians.y; // Yaw (Y-axis)
    const float z = eulerRadians.z; // Roll (Z-axis)

    // Left-handed rotation matrices
    // X-axis rotation (pitch)
    const float3x3 rotX = float3x3(
        1, 0, 0,
        0, cos(x), sin(x),
        0, -sin(x), cos(x)
    );

    // Y-axis rotation (yaw)
    const float3x3 rotY = float3x3(
        cos(y), 0, -sin(y),
        0, 1, 0,
        sin(y), 0, cos(y)
    );

    // Z-axis rotation (roll)
    const float3x3 rotZ = float3x3(
        cos(z), sin(z), 0,
        -sin(z), cos(z), 0,
        0, 0, 1
    );

    // Combine rotations: Z * Y * X (applied in XYZ order)
    const float3x3 finalRot = mul(rotZ, mul(rotY, rotX));

    // Default forward direction in left-handed systems: positive Z
    const float3 forward = float3(0, 0, 1);

    return normalize(mul(finalRot, forward));
}

float4x4 invertTranspose(float4x4 m)
{
    const float4x4 inverseTransposeMultiplier = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
                                                         0.0f, 1.0f, 0.0f, 0.0f,
                                                         0.0f, 0.0f, 1.0f, 0.0f, 
                                                         0.0f, 0.0f, 0.0f, 1.0f);
    return mul(m, inverseTransposeMultiplier);
}


