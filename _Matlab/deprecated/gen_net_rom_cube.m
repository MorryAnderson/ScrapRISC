clear all;

%% 输入输出
in_file_path = "../../_Logisim/ROM/led.irom";
out_file_path = "./net/led_cube.net";

%% 常量
MAX_CONNECTIONS_PER_GATE = 128;
WIDTH = 32;
D_X = 8;
D_Y = 16;
DEPTH = D_X * D_Y;
X_OFFSET = 2;
EOP = 0xFFFF;

ADDR_WIDTH = log2(DEPTH);
% GATE_COUNT = 2*ADDR_WIDTH + DEPTH + DEPTH * WIDTH + WIDTH + 1;


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
    if (numel(line) < 2)
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
rom_data = rot90(rom_data);

fclose(fid);


%% ID分配
num_input = 2*ADDR_WIDTH;
num_index = DEPTH;
num_data = DEPTH * WIDTH;
num_output = WIDTH;
num_gnd = DEPTH * WIDTH / MAX_CONNECTIONS_PER_GATE;

id_input = (1:num_input);
id_index = (1:num_index) + id_input(end);
id_data = (1:num_data) + id_index(end);
id_output = (1:num_output) + id_data(end);
id_gnd = (1:num_gnd) + id_output(end);

gates_input = Gate.empty(num_input, 0);
gates_index = Gate.empty(num_index, 0);
gates_data = Gate.empty(num_data, 0);
gates_output = Gate.empty(num_output, 0);
gates_gnd = Gate.empty(num_gnd, 0);

num_dest = 0;


%% 输入
orientation = Gate.FACE_LEFT;
color = Gate.COLOR_INPUT;
x = -2;
index_bin = fliplr(decimalToBinaryVector(0:DEPTH-1));

for n=1:num_input/2
    for b=1:2
        y = 2*(n-1);
        z = b-1;
        i = y + b;
        id = id_input(i);

        if (z==0)
            mode = Gate.MODE_NOR; 
        else
            mode = Gate.MODE_OR;
        end
    
        dest = id_index(index_bin(:,n)==z);
        gates_input(i) = Gate(id, mode, orientation, x+X_OFFSET, -y, z, color, dest);
    end
end


%% 序号
mode = Gate.MODE_AND;
orientation = Gate.FACE_UP;
color = Gate.COLOR_DEFAULT;
z = WIDTH;
i = 0;

for x = 0:(D_X-1)
    for y = 0:(D_Y-1)
        i = i+1;
        id = id_index(i); 
        gates_index(i) = Gate(id, mode, orientation, x+X_OFFSET, -y, z, color);
    end
end


%% 数据
mode = Gate.MODE_AND;
orientation = Gate.FACE_UP;
color = Gate.COLOR_DEFAULT;
i = 0;
d = 0;
n = 0;

for x = 0:(D_X-1)
    for y = 0:(D_Y-1)
        d = d + 1;
        dest = id_data((d-1)*WIDTH+1:d*WIDTH);
        gates_index(d).destination = dest;
        for z = 1:WIDTH
            i = i+1;
            id = id_data(i); 
            gates_data(i) = Gate(id, mode, orientation, x+X_OFFSET, -y, z-1, color, id_output(z));
        end
    end
end


%% GND
mode = Gate.MODE_AND;
orientation = Gate.FACE_LEFT;
color = Gate.COLOR_DEFAULT;
x = -1;

for i = 1:num_gnd
    id = id_gnd(i);
    y = mod(i-1,D_Y);
    z = ceil(i/D_Y)-1;
    gates_gnd(i) = Gate(id, mode, orientation, x+X_OFFSET, -y, z, color);
end

for i=1:min(numel(rom_data),DEPTH*WIDTH)
    n = ceil(i/MAX_CONNECTIONS_PER_GATE);
    if (rom_data(i) == 0)
       gates_gnd(n).destination = [gates_gnd(n).destination, id_data(i)]; 
    end
end


%% 输出
mode = Gate.MODE_OR;
orientation = Gate.FACE_RIGHT;
color = Gate.COLOR_OUTPUT;
x = D_X;
y = 0;
dest = zeros(0, 'uint16');

for z = 1:WIDTH
    id = id_output(z); 
    gates_output(z) = Gate(id, mode, orientation, x+X_OFFSET, y, z-1, color, dest);
end

%%
gates = [gates_input,gates_gnd,gates_index,gates_data,gates_output];

NetCheck(gates);


%% 写入net文件
fid = fopen(out_file_path, 'wb'); % 'wb' 表示以二进制写入模式打开文件
if fid == -1
    error('无法创建文件。');
end

% 文件头
NetWriteFilehead(fid, D_X+3, D_Y, numel(gates), 2);

% 输入端口
fprintf(fid, '%s', 'I');  % 输入（'I'）
fwrite(fid,ADDR_WIDTH,'uint8');  % 端口位宽
fprintf(fid, '%s\0', 'I_adr');  % 端口名称，以\0结束

% 输入端口连接
for i=1:num_input/2
    fwrite(fid,i-1,'uint8');  % 端口的第几位（从0开始)
    fwrite(fid,2,'uint8');  % 该位连接了几个逻辑门（N）
    for b=1:2
        fwrite(fid,id_input(2*(i-1)+b),'uint16');  % 逻辑门id    
    end
end
fwrite(fid,EOP,'uint16');  % 帧尾

% 输出端口
fprintf(fid, '%s', 'O');  % 输出（'O'）
fwrite(fid,WIDTH,'uint8');  % 端口位宽
fprintf(fid, '%s\0', 'O_dat');  % 端口名称，以\0结束

% 输出端口连接
for i=1:num_output
    fwrite(fid,i-1,'uint8');  % 该门为端口的第几位（从0开始)
    fwrite(fid,1,'uint8');  % 该位连接了几个逻辑门（N）
    fwrite(fid,id_output(i),'uint16');  % 逻辑门id  
end
fwrite(fid,EOP,'uint16');  % 帧尾

% 拓扑数据
NetWriteGates(fid, gates);


fclose(fid);
