# 获取文件夹路径
$folderPath = "../../src/hdl"

# 定义文件扩展名
$cppExtensions = @(".scrp")

# 定义要排除的文件名
$excludedFileName = "debug.srcp"

# 初始化行数计数器
$totalLines = 0

# 遍历文件夹下所有文件
Get-ChildItem -Path $folderPath -Recurse -File | ForEach-Object {
    # 检查文件扩展名是否为 C++ 文件，且文件名不是要排除的文件
    if ($cppExtensions -contains $_.Extension -and $_.Name -ne $excludedFileName) {
        # 读取文件内容并统计行数
        $lines = Get-Content $_.FullName | Measure-Object -Line | Select-Object -ExpandProperty Lines
        Write-Host ("File: {0}, Lines: {1}" -f $_.FullName, $lines)

        # 累加行数到总计
        $totalLines += $lines
    }
}

# 输出总行数
Write-Host ("Total Lines: {0}" -f $totalLines)

# 等待用户按下任意键以继续
Read-Host "Press Enter to continue"
