Get-VSSetupInstance `
    | Select-VSSetupInstance -Latest `
    | Select-Object -ExpandProperty InstallationPath `
    | Set-Variable -Name installationPath

& "${env:COMSPEC}" /s /c "`"$installationPath\VC\Auxiliary\Build\vcvarsall.bat`" x64 > nul && set" | foreach-object {
    $name, $value = $_ -split '=', 2
    set-content env:\"$name" $value
}