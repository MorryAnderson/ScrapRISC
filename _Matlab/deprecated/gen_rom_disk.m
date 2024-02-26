clear all;

%% 输入输出
in_file_path = "../../_Logisim/ROM/snake.irom";
out_folder_path = "./blueprints";


%% 常量
ICON_PATH = './icon.png'; % 默认图片路径
BLOCK_SHAPE_ID = '628b2d61-5ceb-43e9-8334-a4135566df7a';
WIDTH = 32;
COLOR_ZERO = '222222';
COLOR_ONE = 'EEEEEE';
COLOR_ROOT = 'EE0000';


%% 检查文件格式
fid = fopen(in_file_path, 'rt');
if fid == -1
    error('无法打开文件');
end

header = fgetl(fid);
if ~strcmp(header, 'v2.0 raw')
    fclose(fid);
    error('文件头不匹配');
end


%% 读取数据
num_lines  = 0;
while ~feof(fid)
    line = fgetl(fid);
    if (numel(line) ~= 8)
        break;
    else
        num_lines  = num_lines  + 1;
    end
end

frewind(fid);
fgetl(fid);

rom_data  = zeros(num_lines-1, WIDTH);

for n = 1:num_lines
    hex_string  = fgetl(fid);
    rom_data(n,:)  = hexToBinaryVector(hex_string,WIDTH);
end

fclose(fid);

%% 创建蓝图文件
[~, file_name, ~] = fileparts(in_file_path);
fid = BpCreateFolder(file_name,out_folder_path,ICON_PATH);
BpWriteFileHead(fid);

for i = 1:num_lines
    z = i-1;
    for n = 1:WIDTH
        x = n-1;
        if (rom_data(i,n)==1)
            color = COLOR_ONE;
        else
            color = COLOR_ZERO;
        end
        BpCreateBlock(fid, BLOCK_SHAPE_ID,color, x,0,z);
    end
end

BpCreateBlock(fid, BLOCK_SHAPE_ID ,COLOR_ROOT, -1,0,0,0);

BpWriteFileTail(fid);

fclose(fid);

