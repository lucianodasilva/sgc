param (
    [string]$cmake_args = "",
    [string]$arch = ""
)

Write-Host "Setup ..."

if ($isLinux) {
    if ($arch -eq '32') {
        Write-Host "Install Multilib for 32Bit Linux"
        sudo apt-get update
        sudo apt-get install g++-7-multilib -y
    }
}

$starting_location = (Get-Location).toString()
$build_folder = "build"
$tests_folder = "tests"

if (Test-Path $build_folder) {
    Remove-Item $build_folder -Recurse -Confirm:$false -ErrorAction SilentlyContinue -Force
}

New-Item -ItemType directory $build_folder | Out-Null
Set-Location $build_folder

Write-Host "Building ..."

# setup make
Invoke-Expression "cmake .. $cmake_args -DCMAKE_BUILD_TYPE=Release -DSGC_UNIT_TESTS=ON"
# compile
cmake --build . --config Release

# test
Set-Location $tests_folder

if (Test-Path out) {
    Remove-Item out -Recurse -Confirm:$false -Force
}

New-Item -ItemType directory out | Out-Null

Write-Host "Testing ..."

# remove manifest files
Get-ChildItem -Recurse -Filter *.manifest -File | ForEach-Object {
    Remove-Item $_.FullName
}

Get-ChildItem . -Filter * -File | ForEach-Object {
    $xmlfile = "out/$($_.BaseName).xml"
    Invoke-Expression "./$($_.BaseName) -r junit" | Out-File -FilePath $xmlfile
}

# restore path
Set-Location $starting_location

Write-Host "Done!"
exit 0