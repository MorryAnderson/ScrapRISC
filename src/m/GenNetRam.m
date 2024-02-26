function GenNetRam(out_file_path,WIDTH,DEPTH,DUAL_RD,BYTE_EN,REG0)

%% constants
ADDR_WIDTH = log2(DEPTH);
BYTE_EN_VLD = BYTE_EN && WIDTH > 0 && mod(WIDTH,8)==0;
BYTE_COUNT = WIDTH/8;


%% inputs
id = 1;
y = 0;

[gate_input_clk     , id, y] = GatesInput(id,0,y,1);
[gate_input_rstn    , id, y] = GatesInput(id,0,y,1);
[gate_input_rd_adr_1, id, y] = GatesInput(id,0,y,ADDR_WIDTH);
if (DUAL_RD)
    [gate_input_rd_adr_2, id, y] = GatesInput(id,0,y,ADDR_WIDTH);
end

[gate_input_wr_adr  , id, y] = GatesInput(id,0,y,ADDR_WIDTH);
[gate_input_wr_dat  , id, y] = GatesInput(id,0,y,WIDTH);
[gate_input_wr_ena  , id, y] = GatesInput(id,0,y,1);

if (BYTE_EN_VLD)
    [gate_input_be      , id, y] = GatesInput(id,0,y,BYTE_COUNT);
end



%% fan buffer
mode = Gate.MODE_OR;
orientation = Gate.FACE_LEFT;
color = Gate.COLOR_DEFAULT;

x = gate_input_clk.x + 1;
y = gate_input_clk.y;
gate_fan_clk  = Gate.empty(DEPTH,0);
id_fan_clk = zeros(DEPTH,1);

for i = 1:DEPTH
    gate_fan_clk(i) = Gate(id, mode, orientation, x, y, i-1, color);
    id_fan_clk(i) = id;
    id = id + 1;
end
gate_input_clk.destination = id_fan_clk;


x = gate_input_rstn.x + 1;
y = gate_input_rstn.y;
gate_fan_rstn = Gate.empty(DEPTH,0);
id_fan_rstn = zeros(DEPTH,1);

for i = 1:DEPTH
    gate_fan_rstn(i) = Gate(id, mode, orientation, x, y, i-1, color);
    id_fan_rstn(i) = id;
    id = id + 1;
end
gate_input_rstn.destination = id_fan_rstn;


%% sync rstn

gate_input_rstn.mode = Gate.MODE_NOR;  % srn = nand(clk, not(arn))
[gate_fan_rstn.mode] = deal(Gate.MODE_NAND);
gate_input_clk.destination = [gate_input_clk.destination, id_fan_rstn];


%% pn buffer
orientation = Gate.FACE_FRONT;
color = Gate.COLOR_DEFAULT;
x = gate_input_rd_adr_1(1).x;
y = gate_input_rd_adr_1(1).y;

gate_buffer_p_rd_adr_1 = Gate.empty(ADDR_WIDTH, 0);
gate_buffer_n_rd_adr_1 = Gate.empty(ADDR_WIDTH, 0);
for i = 1:ADDR_WIDTH
    gate_input_rd_adr_1(i).destination = [id, id+1];
    
    gate_buffer_p_rd_adr_1(i) = Gate(id, Gate.MODE_OR, orientation, x+1, y, i-1, color);
    id = id + 1;
    gate_buffer_n_rd_adr_1(i) = Gate(id, Gate.MODE_NOR, orientation, x+2, y, i-1, color);
    id = id + 1;
end


if (DUAL_RD)  %###

    x = gate_input_rd_adr_2(1).x;
    y = gate_input_rd_adr_2(1).y;

    gate_buffer_p_rd_adr_2 = Gate.empty(ADDR_WIDTH, 0);
    gate_buffer_n_rd_adr_2 = Gate.empty(ADDR_WIDTH, 0);
    for i = 1:ADDR_WIDTH
        gate_input_rd_adr_2(i).destination = [id, id+1];
        
        gate_buffer_p_rd_adr_2(i) = Gate(id, Gate.MODE_OR, orientation, x+1, y, i-1, color);
        id = id + 1;
        gate_buffer_n_rd_adr_2(i) = Gate(id, Gate.MODE_NOR, orientation, x+2, y, i-1, color);
        id = id + 1;
    end

end  %###


gate_buffer_p_wr_adr = Gate.empty(ADDR_WIDTH, 0);
gate_buffer_n_wr_adr = Gate.empty(ADDR_WIDTH, 0);
for i = 1:ADDR_WIDTH
    x = gate_input_wr_adr(i).x;
    y = gate_input_wr_adr(i).y;
    gate_input_wr_adr(i).destination = [id, id+1];
    
    gate_buffer_p_wr_adr(i) = Gate(id, Gate.MODE_OR, orientation, x+1, y, i-1, color);
    id = id + 1;
    gate_buffer_n_wr_adr(i) = Gate(id, Gate.MODE_NOR, orientation, x+2, y, i-1, color);
    id = id + 1;
end

gate_buffer_p_wr_dat = Gate.empty(WIDTH, 0);
gate_buffer_n_wr_dat = Gate.empty(WIDTH, 0);
for i = 1:WIDTH
    x = gate_input_wr_dat(i).x;
    y = gate_input_wr_dat(i).y;
    gate_input_wr_dat(i).destination = [id, id+1];
    
    gate_buffer_p_wr_dat(i) = Gate(id, Gate.MODE_OR, orientation, x+1, y, i-1, color);
    id = id + 1;
    gate_buffer_n_wr_dat(i) = Gate(id, Gate.MODE_NOR, orientation, x+2, y, i-1, color);
    id = id + 1;
end


%% demux
mode = Gate.MODE_AND;
orientation = Gate.FACE_FRONT;
color = Gate.COLOR_WHITE;
x = gate_input_wr_adr(1).x + 3;
y = gate_input_wr_adr(1).y;

index_bin = fliplr(decimalToBinaryVector(0:DEPTH-1));

gate_mux_wr_adr = Gate.empty(DEPTH, 0);
id_mux_wr_adr = zeros(DEPTH,1);

for i = 1:DEPTH
    gate_mux_wr_adr(i) = Gate(id, mode, orientation, x, y, i-1, color);
    id_mux_wr_adr(i) = id;
    id = id + 1;
end

for i = 1:ADDR_WIDTH
    gate_buffer_p_wr_adr(i).destination = id_mux_wr_adr(index_bin(:,i)==1);
    gate_buffer_n_wr_adr(i).destination = id_mux_wr_adr(index_bin(:,i)==0);
end

gate_input_wr_ena.destination = id_mux_wr_adr;
x_offset = gate_mux_wr_adr(1).x + 1;


%% mux
mode = Gate.MODE_AND;
orientation = Gate.FACE_RIGHT;
color = Gate.COLOR_DEFAULT;

x = gate_input_rd_adr_1(1).x + 3;
y = gate_input_rd_adr_1(1).y;

gate_mux_rd_adr_1 = Gate.empty(DEPTH, 0);
id_mux_rd_adr_1 = zeros(DEPTH,1);

for i = 1:DEPTH
    gate_mux_rd_adr_1(i) = Gate(id, mode, orientation, x, y, i-1, color);
    id_mux_rd_adr_1(i) = id;
    id = id + 1;
end

for i = 1:ADDR_WIDTH
    gate_buffer_p_rd_adr_1(i).destination = id_mux_rd_adr_1(index_bin(:,i)==1);
    gate_buffer_n_rd_adr_1(i).destination = id_mux_rd_adr_1(index_bin(:,i)==0);
end

if (DUAL_RD) %###

    x = gate_input_rd_adr_2(1).x + 3;
    y = gate_input_rd_adr_2(1).y;

    gate_mux_rd_adr_2 = Gate.empty(DEPTH, 0);
    id_mux_rd_adr_2 = zeros(DEPTH,1);

    for i = 1:DEPTH
        gate_mux_rd_adr_2(i) = Gate(id, mode, orientation, x, y, i-1, color);
        id_mux_rd_adr_2(i) = id;
        id = id + 1;
    end

    for i = 1:ADDR_WIDTH
        gate_buffer_p_rd_adr_2(i).destination = id_mux_rd_adr_2(index_bin(:,i)==1);
        gate_buffer_n_rd_adr_2(i).destination = id_mux_rd_adr_2(index_bin(:,i)==0);
    end
    
end %###


%% mux (part 2)
mode = Gate.MODE_AND;
orientation = Gate.FACE_BACK;
color = Gate.COLOR_DEFAULT;
y = 0;

gate_mux_rd_1 = Gate(DEPTH,WIDTH);
id_mux_rd_1 = zeros(DEPTH,WIDTH);

for i = 1:DEPTH
    for n = 1:WIDTH
        x = WIDTH-1 - (n-1) + x_offset;
        gate_mux_rd_1(i,n) = Gate(id, mode, orientation, x, y, i-1, color);
        id_mux_rd_1(i,n) = id;
        id = id + 1;
    end
end

for i = 1:DEPTH
    gate_mux_rd_adr_1(i).destination = id_mux_rd_1(i,:);
end


if (DUAL_RD) %###

    y = -1;

    gate_mux_rd_2 = Gate(DEPTH,WIDTH);
    id_mux_rd_2 = zeros(DEPTH,WIDTH);
    
    for i = 1:DEPTH
        for n = 1:WIDTH
            x = WIDTH-1 - (n-1) + x_offset;
            gate_mux_rd_2(i,n) = Gate(id, mode, orientation, x, y, i-1, color);
            id_mux_rd_2(i,n) = id;
            id = id + 1;
        end
    end
    
    for i = 1:DEPTH
        gate_mux_rd_adr_2(i).destination = id_mux_rd_2(i,:);
    end

end %###


%% latch
gate_latch_nand_dp = Gate(DEPTH,WIDTH);
gate_latch_nand_dn = Gate(DEPTH,WIDTH);
gate_latch_nand_qp = Gate(DEPTH,WIDTH);
gate_latch_nand_qn = Gate(DEPTH,WIDTH);
gate_latch_and_buf = Gate(DEPTH,WIDTH);

data_id = reshape((1:DEPTH*WIDTH*5)+id,DEPTH,WIDTH,[]);

id_array_data_nand_dp = data_id(:,:,1);
id_array_data_nand_dn = data_id(:,:,2);
id_array_data_nand_qp = data_id(:,:,3);
id_array_data_nand_qn = data_id(:,:,4);
id_array_data_and_buf = data_id(:,:,5);
id = data_id(end,end,end)+1;


orientation = Gate.FACE_FRONT;
color = Gate.COLOR_DEFAULT;

if (DUAL_RD) %###
    y_offset = -2;
else
    y_offset = -1;
end %###

for i = 1:DEPTH
    for n = 1:WIDTH
        x = WIDTH-1 - (n-1) + x_offset;
        gate_latch_nand_dp(i,n) = Gate(id_array_data_nand_dp(i,n), Gate.MODE_NAND, orientation, x, y_offset-0, i-1, color, id_array_data_nand_qp(i,n));  
        gate_latch_nand_dn(i,n) = Gate(id_array_data_nand_dn(i,n), Gate.MODE_NAND, orientation, x, y_offset-1, i-1, color, id_array_data_nand_qn(i,n)); 
        gate_latch_nand_qn(i,n) = Gate(id_array_data_nand_qn(i,n), Gate.MODE_NAND, orientation, x, y_offset-2, i-1, color, id_array_data_and_buf(i,n)); 
        gate_latch_and_buf(i,n) = Gate(id_array_data_and_buf(i,n), Gate.MODE_AND,  orientation, x, y_offset-3, i-1, color, id_array_data_nand_qp(i,n));
        gate_latch_nand_qp(i,n) = Gate(id_array_data_nand_qp(i,n), Gate.MODE_NAND, orientation, x, y_offset-4, i-1, color, id_array_data_nand_qn(i,n));  
    end
end

% connection
for i = 1:DEPTH
    gate_fan_clk(i).destination = [id_array_data_nand_dp(i,:), id_array_data_nand_dn(i,:)];
    gate_fan_rstn(i).destination = [id_array_data_nand_dp(i,:), id_array_data_nand_qn(i,:)];
end

for i = 1:DEPTH
    gate_mux_wr_adr(i).destination = [id_array_data_nand_dp(i,:), id_array_data_nand_dn(i,:)];
end

for n = 1:WIDTH
    gate_buffer_p_wr_dat(n).destination = id_array_data_nand_dp(:,n);
    gate_buffer_n_wr_dat(n).destination = id_array_data_nand_dn(:,n);
end

for i = 1:DEPTH
    for n = 1:WIDTH
        gate_latch_nand_qp(i,n).destination = [id_array_data_nand_qn(i,n), id_mux_rd_1(i,n)];
        if (DUAL_RD) %###
            gate_latch_nand_qp(i,n).destination = [gate_latch_nand_qp(i,n).destination, id_mux_rd_2(i,n)];
        end %###
    end
end


%% output
mode = Gate.MODE_OR;
orientation = Gate.FACE_RIGHT;
color = Gate.COLOR_OUTPUT;
x = x_offset + WIDTH;
y = 0;


gates_dout_1 = Gate.empty(WIDTH,0);

for n = 1:WIDTH
    gates_dout_1(n) = Gate(id, mode, orientation, x, y, n-1, color);
    id = id + 1;
end

for i = 1:DEPTH
    for n = 1:WIDTH
        gate_mux_rd_1(i,n).destination = gates_dout_1(n).id;
    end
end

if (DUAL_RD) %###

    y = -1;

    gates_dout_2 = Gate.empty(WIDTH,0);
    
    for n = 1:WIDTH
        gates_dout_2(n) = Gate(id, mode, orientation, x, y, n-1, color);
        id = id + 1;
    end
    
    for i = 1:DEPTH
        for n = 1:WIDTH
            gate_mux_rd_2(i,n).destination = gates_dout_2(n).id;
        end
    end

end %###


%% reg0
if (REG0)
    for n = 1:WIDTH
        gate_latch_nand_dp(1,n).destination = [];
        gate_latch_and_buf(1,n).destination = [];
        gate_latch_nand_qp(1,n).mode = Gate.MODE_AND;
    end
end


%% byte enable
if (BYTE_EN_VLD)

    mode = Gate.MODE_OR;
    orientation = Gate.FACE_UP;
    color = Gate.COLOR_DEFAULT;

    x = gate_input_be(1).x + 1;
    y = gate_input_be(1).y;

    M = 8;
    num_fan_be = BYTE_COUNT * ceil(DEPTH/M);
    gate_fan_be  = Gate.empty(num_fan_be,0);
    id_fan_be = zeros(num_fan_be,1);

    for i = 1:num_fan_be/BYTE_COUNT
        for n = 1:BYTE_COUNT
            k = BYTE_COUNT*(i-1)+n;
            range_width = (8*(n-1)+1):(8*n);
            range_depth = (M*(i-1)+1):min((M*i),DEPTH);
            dest1 = reshape(id_array_data_nand_dn(range_depth,range_width),1,[]);
            dest2 = reshape(id_array_data_nand_dp(range_depth,range_width),1,[]);
            gate_fan_be(k) = Gate(id, mode, orientation, x, y, k-1, color, [dest1,dest2]);
            id_fan_be(k) = id;
            id = id + 1;
            gate_input_be(n).destination = [gate_input_be(n).destination, id_fan_be(k)];
        end
    end

end


%% gates
gates_input = [ ...
    gate_input_clk, ...
    gate_input_rstn, ...
    gate_input_rd_adr_1, ...
    gate_input_wr_adr, ...
    gate_input_wr_dat, ...
    gate_input_wr_ena ...
];

gates_output = [gates_dout_1];

gates_fan = [gate_fan_clk, gate_fan_rstn];

gates_buffer = [...
    gate_buffer_p_rd_adr_1, ...
    gate_buffer_n_rd_adr_1, ...
    gate_buffer_p_wr_adr, ...
    gate_buffer_n_wr_adr, ...
    gate_buffer_p_wr_dat, ...
    gate_buffer_n_wr_dat ...
];


gates_mux = [ ...
    gate_mux_wr_adr, ...
    gate_mux_rd_adr_1, ...
    reshape(gate_mux_rd_1,1,[]) ...
];


if (BYTE_EN_VLD)
    gates_input = [gates_input, gate_input_be];
    gates_fan = [gates_fan, gate_fan_be];
end

if (DUAL_RD) %###
    gates_input  = [gates_input, gate_input_rd_adr_2];
    gates_output = [gates_output, gates_dout_2];
    gates_buffer = [gates_buffer, gate_buffer_p_rd_adr_2, gate_buffer_n_rd_adr_2];
    gates_mux    = [gates_mux, gate_mux_rd_adr_2, reshape(gate_mux_rd_2,1,[])];
end %###

gates_latch = [ ...
    reshape(gate_latch_nand_dp,1,[]) , ...
    reshape(gate_latch_nand_dn,1,[]) , ...
    reshape(gate_latch_nand_qp,1,[]) , ...
    reshape(gate_latch_nand_qn,1,[]) , ...
    reshape(gate_latch_and_buf,1,[])   ...
];

gates = [ ...
    gates_input  , ...
    gates_output , ...
    gates_fan    , ...
    gates_buffer , ...
    gates_mux    , ...
    gates_latch
];


if (~NetCheck(gates))
    error('cannot create netlist');
end


%% write net file
fid = fopen(out_file_path, 'wb');
if fid == -1
    error('cannot create file');
end

% file head
INPUT = 0;
OUTPUT = 1;

num_ports = 7;
height = 6;

if (BYTE_EN_VLD)
    num_ports = num_ports + 1;
    height = height + 1;
end
if (DUAL_RD)
    num_ports = num_ports + 2;
    height = height + 1;
end


% file data
NetWriteGates(fid, gates);
NetWritePorthead(fid, WIDTH+x_offset+1,height, num_ports);
NetWritePort(fid,INPUT,'I_clk',gate_input_clk);
NetWritePort(fid,INPUT,'I_rstn',gate_input_rstn);
NetWritePort(fid,INPUT,'I_rd_adr_1',gate_input_rd_adr_1);
NetWritePort(fid,INPUT,'I_wr_adr',gate_input_wr_adr);
NetWritePort(fid,INPUT,'I_din',gate_input_wr_dat);
NetWritePort(fid,INPUT,'I_we',gate_input_wr_ena);
if (BYTE_EN_VLD)
    NetWritePort(fid,INPUT,'I_be',gate_input_be);
end
NetWritePort(fid,OUTPUT,'O_dout_1',gates_dout_1);

if (DUAL_RD)
    NetWritePort(fid,INPUT,'I_rd_adr_2',gate_input_rd_adr_2);
    NetWritePort(fid,OUTPUT,'O_dout_2',gates_dout_2);
end

fclose(fid);


end

