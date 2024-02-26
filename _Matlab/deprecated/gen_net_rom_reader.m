clear all;

%% 输入输出
out_folder_path = "./blueprints";


%% 常量
WIDTH = 32;
DEPTH = 128;
ICON_PATH = './icon.png'; % 默认图片路径
GATE_SHAPE_ID = '9f0f56e8-2c31-4d83-996c-d00a9b296c3f';
SENSOR_SHAPE_ID = '20dcd41c-0a11-4668-9b00-97f278ce21af';
BASE_SHAPE_ID = '628b2d61-5ceb-43e9-8334-a4135566df7a';
BASE_COLOR = '222222';
COLOR_ZERO = '222222';
COLOR_ONE = 'EEEEEE';
COLOR_ROOT = 'EE0000';
X_OFFSET = 2;


%% ID分配
num_input = 2*log2(DEPTH);
num_index = DEPTH;
num_data = DEPTH * WIDTH;
num_output = WIDTH;

id_input = (1:num_input);
id_index = (1:num_index) + id_input(end);
id_data = (1:num_data) + id_index(end);
id_output = (1:num_output) + id_data(end);
id_sensor = (1:num_data) + id_output(end);

gates_input = Gate.empty(num_input, 0);
gates_index = Gate.empty(num_index, 0);
gates_data = Gate.empty(num_data, 0);
gates_output = Gate.empty(num_output, 0);


%% 输入
orientation = Gate.FACE_LEFT;
color = Gate.COLOR_INPUT;
index_bin = fliplr(decimalToBinaryVector(0:DEPTH-1));
y = 0;

for n=1:num_input/2
    for b=1:2
        i = 2*(n-1) + b;
        id = id_input(i);

        if (b==1)
            mode = Gate.MODE_NOR; 
        else
            mode = Gate.MODE_OR;
        end
    
        dest = id_index(index_bin(:,n)==b-1);
        gates_input(i) = Gate(id, mode, orientation, b-1, y, n-1, color, dest);
    end
end


%% 序号
mode = Gate.MODE_AND;
orientation = Gate.FACE_FRONT;
color = Gate.COLOR_DEFAULT;
x = WIDTH+X_OFFSET;
y = -1;

for i = 1:DEPTH
    id = id_index(i); 
    gates_index(i) = Gate(id, mode, orientation, x, y, i-1, color);
end


%% 数据
mode = Gate.MODE_AND;
orientation = Gate.FACE_BACK;
color = Gate.COLOR_DEFAULT;
y = 0;
k = 0;

for i = 1:DEPTH
    dest = id_data((i-1)*WIDTH+1:i*WIDTH);
    gates_index(i).destination = dest;         
    for n = 1:WIDTH
        k = k+1;
        id = id_data(k);
        gates_data(k) = Gate(id, mode, orientation, n-1+X_OFFSET,y,i-1, color,id_output(n));
    end
end


%% 输出
mode = Gate.MODE_OR;
orientation = Gate.FACE_RIGHT;
color = Gate.COLOR_OUTPUT;
x = WIDTH+X_OFFSET;
y = 0;
dest = zeros(0, 'uint16');

for n = 1:WIDTH
    id = id_output(n); 
    gates_output(n) = Gate(id, mode, orientation, x, y, n-1, color, dest);
end


%%
gates = [gates_input,gates_index,gates_data,gates_output];
if (~NetCheck(gates))
    error('创建网表失败');
end

%% 创建蓝图文件
fid = BpCreateFolder("rom_reader",out_folder_path,ICON_PATH);
BpWriteFileHead(fid);

% 逻辑门
for i=1:numel(gates)
    BpCreateGate(fid,gates(i));
end

% 传感器
k = 0;

for i = 1:DEPTH
    for n = 1:WIDTH
        k = k+1;
        fprintf(fid, '{\n');
        fprintf(fid, '\t\"shapeId\":\"%s\",\n',SENSOR_SHAPE_ID);
        fprintf(fid, '\t\"color\":\"%s\",\n',Gate.COLOR_DEFAULT);
        fprintf(fid, '\t\"xaxis\":%d,\n',1);
        fprintf(fid, '\t\"zaxis\":%d,\n',-2);
        fprintf(fid, '\t\"pos\":{\"x\":%d, \"y\":%d, \"z\":%d},\n',n-1+X_OFFSET,-1+1,i-1);
        fprintf(fid, '\t\"controller\":{\n');
            fprintf(fid, '\t\t\"id\":%d,\n',id_sensor(k));  
            fprintf(fid, '\t\t\"range\":1,\n');
            fprintf(fid, '\t\t\"joints\":null,\n');
            fprintf(fid, '\t\t\"audioEnabled\":false,\n');
            fprintf(fid, '\t\t\"buttonMode\":true,\n');
            fprintf(fid, '\t\t\"colorMode\":true,\n');
            fprintf(fid, '\t\t\"color\":\"%s\",\n',COLOR_ONE);
            fprintf(fid, '\t\t\"controllers\":[\n');
                fprintf(fid, '\t\t\t{\"id\":%d}',id_data(k));
                fprintf(fid, '\n');
            fprintf(fid, '\t\t]\n');
        fprintf(fid, '\t}\n');
        fprintf(fid, '},\n');
    end
end

% 插口
BpCreateBlock(fid, BASE_SHAPE_ID,COLOR_ROOT,0,-2,0);
BpCreateBlock(fid, BASE_SHAPE_ID,COLOR_ROOT,0,-2,1);
BpCreateBlock(fid, BASE_SHAPE_ID,COLOR_ROOT,1,-2,1);

% 基底
BpCreateBase(fid, BASE_SHAPE_ID, BASE_COLOR, WIDTH + X_OFFSET + 1, 3);
BpWriteFileTail(fid);

fclose(fid);