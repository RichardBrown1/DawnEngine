# Define paths
$shaderSourcePath = ".\"
#$compiledShadersPath = "..\out\build\x64-debug\shaders"
$compiledShadersPath = ".\"
$dxcPath = "C:\VulkanSDK\1.3.268.0\Bin\dxc.exe"

# Create compiled shaders directory if it doesn't exist
if (-Not (Test-Path -Path $compiledShadersPath)) {
    New-Item -ItemType Directory -Path $compiledShadersPath
}

# Compile shaders
Get-ChildItem -Path $shaderSourcePath -Filter *.hlsl | ForEach-Object {
    $shortName = $_.Name -replace "\..*", "" 
    echo "compiling $shortname"
    & $dxcPath -T vs_6_7 -E VS_main -spirv -enable-16bit-types -Fo $compiledShadersPath/v_$shortName.spv $_.FullName
    & $dxcPath -T ps_6_7 -E FS_main -spirv -enable-16bit-types -Fo $compiledShadersPath/f_$shortName.spv $_.FullName
}
