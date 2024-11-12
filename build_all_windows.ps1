function Get-Build-Version {
    param (
        $Path
    )

    $File = Get-Content -Path $Path
    $Result = ($File | Select-String -Pattern '^### (\d+\.\d+\.\d+)').Matches
    
    if ($Result.Success) {
        return $Result.Groups[1]
    }
    
    Write-Error -Message "Invalid Version." -Category InvalidData
}

$Version = Get-Build-Version -Path "./README.md"
$ZipPrefixPattern = "gx-miscellaneous-$Version{0}-x86_"

Write-Output "Detected Version: $Version"

function Build-Preset {
    param (
        $PresetName,
        $Extra
    )

    $PlatformPresetName = $PresetName -ireplace '-debug|-release', '-windows$0'
    $ZipPrefix = $ZipPrefixPattern -f $Extra
    $PresetFolder = "$PresetName$Extra-user"
    $BuildDir = "./out/build/$PresetFolder"
    $InstallDir = "./out/install/$PresetFolder"
    $ZipFolder = "$ZipPrefix$PlatformPresetName"
    $ZipDir = "$InstallDir/../$ZipFolder"
    $ZipFile = "$InstallDir/../$ZipFolder.zip"
    
    cmake --preset $PresetName
    cmake --build $BuildDir --target install
    Rename-Item -Path $InstallDir -NewName $ZipFolder -Force
    Compress-Archive -Path $ZipDir -DestinationPath $ZipFile -Force
    Rename-Item -Path $ZipDir -NewName $PresetFolder -Force
}

Build-Preset -PresetName x64-debug
Build-Preset -PresetName x64-release
Build-Preset -PresetName x64-debug -Extra -no-jni
Build-Preset -PresetName x64-release -Extra -no-jni
