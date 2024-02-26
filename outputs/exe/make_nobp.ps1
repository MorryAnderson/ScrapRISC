$curr_path = $MyInvocation.MyCommand.Path
$scriptDirectory = Split-Path $curr_path -Parent
Set-Location $scriptDirectory


$exe_path = "./ScrapHDLCompiler_nobp.exe"
$log_path = "./log_nobp.txt"
$m_function = "Net2Blueprint"
$script_path = "../../src/m"


$makefile_path = "../../src/hdl/Makefile.make"
$net_path = "../../outputs/temp"
$bp_path = "../../outputs/temp"


$file_content = Get-Content -Path $makefile_path

foreach ($line in $file_content) {
    if ($line -match "TOP\s*=\s*(.*)") {
        $top_name = $Matches[1].Trim()
        break
    }
}

if (-not $top_name) {
    $top_name = "M3_top"
}

Write-Host "compiling hdl ..."
$process = Start-Process -NoNewWindow -Wait -PassThru -FilePath $exe_path -ArgumentList $makefile_path
$exit_code = $process.ExitCode

if (exit_code -eq 0) {
    Write-Host "compilation complete"
    Write-Host "loading netlist ..."
    matlab -batch "addpath('$script_path'); $m_function('$net_path/$top_name.net', '$bp_path'); exit;"
} else {
    Write-Host "compiler exit with code: $exit_code"
    Write-Host "compilation failed, see $log_path"
    Get-Content -Path $log_path
}

Read-Host "Press Enter to continue ..."
