function GenNetRomPlane(irom_file_path,net_file_path,DEPTH, shrink_to_fit)

if (nargin < 4)
    shrink_to_fit = 0;
end


%% constant
WIDTH = 32;
X_OFFSET = 2;
Y_OFFSET = -1;
ADDR_WIDTH = log2(DEPTH);


%% check file format
fid = fopen(irom_file_path, 'rt');
if fid == -1
    error('cannot open file');
end

header = fgetl(fid);
if ~strcmp(header, 'v2.0 raw')
    fclose(fid);
    error('file head dismatch');
end


%% read data
num_lines  = 0;
word_len = 0;

while ~feof(fid)
    line = fgetl(fid);
    len = numel(line);
    if (word_len == 0)
        word_len = len*4;
    end
    if (len == 8 || len == 2)
        num_lines  = num_lines  + 1;
    elseif (len == 0)
    	break;
    else
        error('unsupported word length');
    end
end

frewind(fid);
fgetl(fid);

if (word_len == 32)
    rom_data  = zeros(num_lines, WIDTH);
    for n = 1:num_lines
        hex_string  = fgetl(fid);
        rom_data(n,:)  = hexToBinaryVector(hex_string,WIDTH);
    end
elseif (word_len == 8)
    if (mod(num_lines,4)~=0)
        error('number of lines must be multiple of 4');
    else
        rom_data  = zeros(num_lines/4, WIDTH);
        for n = 1:num_lines/4
            byte_0  = fgetl(fid);
            byte_1  = fgetl(fid);
            byte_2  = fgetl(fid);
            byte_3  = fgetl(fid);
            hex_string = [byte_3, byte_2, byte_1, byte_0];
            rom_data(n,:)  = hexToBinaryVector(hex_string,WIDTH);
        end
    end
else
    error('unsupported word length');
end

fclose(fid);

if (shrink_to_fit)
    num_rows = num_lines;
else
    num_rows = DEPTH;
end


%% ID allocation
num_input = ADDR_WIDTH;
num_input_pn = 2*ADDR_WIDTH;
num_index = DEPTH;
num_data = DEPTH * WIDTH;
num_output = WIDTH;
num_gnd = WIDTH;

id_input = (1:num_input);
id_input_pn = (1:num_input_pn) + id_input(end);
id_index = (1:num_index) + id_input_pn(end);
id_data = (1:num_data) + id_index(end);
id_output = (1:num_output) + id_data(end);
id_gnd = (1:num_gnd) + id_output(end);

gates_input = Gate.empty(num_input, 0);
gates_input_pn = Gate.empty(num_input_pn, 0);
gates_index = Gate.empty(num_index, 0);
gates_data = Gate.empty(num_data, 0);
gates_output = Gate.empty(num_output, 0);
gates_gnd = Gate.empty(num_gnd, 0);


%% input
mode = Gate.MODE_OR;
orientation = Gate.FACE_LEFT;
color = Gate.COLOR_INPUT;
x = X_OFFSET-2;
y = Y_OFFSET+1;

for i=1:num_input
    id = id_input(i);
    dest = id_input_pn(2*i-1 : 2*i);
    gates_input(i) = Gate(id, mode, orientation, x, y, i-1, color, dest);
end


%% input pn
orientation = Gate.FACE_FRONT;
color = Gate.COLOR_DEFAULT;
index_bin = fliplr(decimalToBinaryVector(0:DEPTH-1));
x = X_OFFSET-2;
y = Y_OFFSET;

for n=1:num_input
    for b=1:2
        i = 2*(n-1) + b;
        id = id_input_pn(i);

        if (b==1)
            mode = Gate.MODE_NOR; 
        else
            mode = Gate.MODE_OR;
        end
    
        dest = id_index(index_bin(1:num_rows,n)==b-1);
        gates_input_pn(i) = Gate(id, mode, orientation, x, y, i-1, color, dest);
    end
end


%% index
mode = Gate.MODE_AND;
orientation = Gate.FACE_FRONT;
color = Gate.COLOR_WHITE;
x = X_OFFSET-1;
y = Y_OFFSET;

for i = 1:num_rows
    id = id_index(i); 
    gates_index(i) = Gate(id, mode, orientation, x, y, i-1, color);
end


%% data
mode = Gate.MODE_AND;
orientation = Gate.FACE_FRONT;
color = Gate.COLOR_DEFAULT;
y = Y_OFFSET;
k = 0;

for i = 1:num_rows
    dest = id_data((i-1)*WIDTH+1:i*WIDTH);
    gates_index(i).destination = dest;         
    for n = 1:WIDTH
        k = k+1;
        id = id_data(k);
        x = n-1+X_OFFSET;
        gates_data(k) = Gate(id, mode, orientation, x,y,i-1, color,id_output(WIDTH-n+1));
    end
end


%% GND
mode = Gate.MODE_AND;
orientation = Gate.FACE_FRONT;
color = Gate.COLOR_WHITE;
y = Y_OFFSET;
z = num_rows;

for i = 1:num_gnd
    id = id_gnd(i);
    gates_gnd(i) = Gate(id, mode, orientation, i-1+X_OFFSET, y, z, color);
end

k = 0;
for i=1:min(size(rom_data,1),num_rows)  
    for n = 1:WIDTH
        k = k+1;
        if (rom_data(i,n) == 0)
           gates_gnd(n).destination = [gates_gnd(n).destination, id_data(k)]; 
        end
    end
end


%% output
mode = Gate.MODE_OR;
orientation = Gate.FACE_UP;
color = Gate.COLOR_OUTPUT;
x = X_OFFSET;
y = Y_OFFSET+1;
z = 0;
dest = zeros(0, 'uint16');

for n = 1:WIDTH
    id = id_output(n); 
    gates_output(n) = Gate(id, mode, orientation, x+WIDTH-n, y, z, color, dest);
end


%%
gates = [gates_input,gates_input_pn,gates_index,gates_data,gates_output,gates_gnd];
if (~NetCheck(gates))
    error('cannot create netlist');
end


%% write net file

fid = fopen(net_file_path, 'wb');
if fid == -1
    error('cannot create file');
end

INPUT = 0;
OUTPUT = 1;

NetWriteGates(fid, gates);
NetWritePorthead(fid, X_OFFSET+WIDTH, 2, 2);
NetWritePort(fid, INPUT, 'I_adr' ,gates_input);
NetWritePort(fid, OUTPUT, 'O_dat' ,gates_output);


fclose(fid);

end

