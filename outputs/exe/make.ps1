$curr_path = $MyInvocation.MyCommand.Path
$scriptDirectory = Split-Path $curr_path -Parent
Set-Location $scriptDirectory

$exe_path = "./ScrapHDLCompiler.exe"
$log_path = "./log.txt"
$makefile_path = "../../src/hdl/Makefile.make"

Write-Host "compiling hdl ..."
$process = Start-Process -NoNewWindow  -Wait -PassThru -FilePath $exe_path -ArgumentList $makefile_path
$exit_code = $process.ExitCode

if ($exit_code -eq 0) {
    Write-Host "compilation complete"
} else {
    Write-Host "compiler exit with code: $exit_code"
    Write-Host "compilation failed, see $log_path"
    Get-Content -Path $log_path
}

Read-Host "Press Enter to continue ..."
