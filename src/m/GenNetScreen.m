function GenNetScreen(out_file_path)


%% constants
WIDTH = 16;
HEIGHT = 16;
H_ADDR_WIDTH =  log2(HEIGHT);
W_ADDR_WIDTH =  log2(WIDTH);


%%
BUFFER_X = 1;
BUFFER_Y = 13;
SCREEN_X = 1;
SCREEN_Y = 23;


%% inputs
id = 1;
x = 2;
y  = 0;

[gate_input_clk    , id, y] = GatesInput(id,x,y,1);
[gate_input_rstn   , id, y] = GatesInput(id,x,y,1);
[gate_input_din    , id, y] = GatesInput(id,x,y,1);
[gate_input_wr_ena , id, y] = GatesInput(id,x,y,1);
[gate_input_wr_row , id, y] = GatesInput(id,x,y,H_ADDR_WIDTH);
[gate_input_wr_col , id, y] = GatesInput(id,x,y,H_ADDR_WIDTH);
[gate_input_refresh, id, y] = GatesInput(id,x,y,1);
[gate_input_rd_row , id, y] = GatesInput(id,x,y,H_ADDR_WIDTH);
[gate_input_rd_col , id, y] = GatesInput(id,x,y,H_ADDR_WIDTH);
[gate_input_rd_ena , id, y] = GatesInput(id,x,y,1);
num_inputs = abs(y);


%% fan buffer
[gate_fan_clk , id] = GatesFanBuffer(gate_input_clk , HEIGHT, id);
[gate_fan_rstn, id] = GatesFanBuffer(gate_input_rstn, HEIGHT, id);

[gate_fan_din_p, id] = GatesFanBuffer(gate_input_din, HEIGHT, id);
[gate_fan_din_n, id] = GatesFanBuffer(gate_input_din, HEIGHT, id);

[gate_fan_din_n.mode] = deal(Gate.MODE_NOR);
[gate_fan_din_n.x] = deal(gate_fan_din_p(1).x+1);

[gate_fan_we, id] = GatesFanBuffer(gate_input_wr_ena, HEIGHT, id);
[gate_fan_refresh, id] = GatesFanBuffer(gate_input_refresh, HEIGHT, id);


%% sync rstn

gate_input_rstn.mode = Gate.MODE_NOR;  % srn = nand(clk, not(arn))
[gate_fan_rstn.mode] = deal(Gate.MODE_NAND);
id_fan_rstn = [gate_fan_rstn.id];
gate_input_clk.AddDest(id_fan_rstn);


%% demux
[gate_buffer_pn_wr_row, gate_demux_wr_row, id, ~] = GatesDemux(gate_input_wr_row, id, 1);
[gate_buffer_pn_wr_col, gate_demux_wr_col, id, ~] = GatesDemux(gate_input_wr_col, id, 1);

[gate_demux_wr_row.orientation] = deal(Gate.FACE_FRONT);
[gate_demux_wr_row.x] = deal(BUFFER_X-1);
[gate_demux_wr_row.y] = deal(-4-BUFFER_Y);

[gate_demux_wr_col.orientation] = deal(Gate.FACE_FRONT);
[gate_demux_wr_col.y] = deal(-4-BUFFER_Y);
[gate_demux_wr_col.z] = deal(HEIGHT);

for n = 1:WIDTH
    gate_demux_wr_col(n).x = BUFFER_X + n-1;
end

gate_fan_we(HEIGHT).orientation = Gate.FACE_FRONT;
gate_fan_we(HEIGHT).x = BUFFER_X-1;
gate_fan_we(HEIGHT).y = -4-BUFFER_Y;
gate_fan_we(HEIGHT).z = HEIGHT;


%% frame_buffer
[gate_latch_frame_buffer_dp, id_latch_frame_buffer_dp, ...
 gate_latch_frame_buffer_dn, id_latch_frame_buffer_dn, ...
 gate_latch_frame_buffer_qp, id_latch_frame_buffer_qp, ...
 gate_latch_frame_buffer_qn, id_latch_frame_buffer_qn, ...
 gate_latch_frame_buffer_buf, ...
 id] = GatesMatrix(WIDTH, HEIGHT, BUFFER_X, -BUFFER_Y, id);

% connection

for i = 1:HEIGHT
    ri = HEIGHT+1 - i;
    id_latch_din = [id_latch_frame_buffer_dp(ri,:), id_latch_frame_buffer_dn(ri,:)];
    gate_fan_clk(i).AddDest(id_latch_din);
    gate_fan_rstn(i).AddDest([id_latch_frame_buffer_dp(ri,:), id_latch_frame_buffer_qn(ri,:)]);
    gate_fan_din_p(i).AddDest(id_latch_frame_buffer_dp(ri,:));
    gate_fan_din_n(i).AddDest(id_latch_frame_buffer_dn(ri,:));
    gate_fan_we(i).AddDest(id_latch_din);
    gate_demux_wr_row(ri).AddDest(id_latch_din);
end

for n = 1:WIDTH
    gate_demux_wr_col(n).AddDest([id_latch_frame_buffer_dp(:,n)', id_latch_frame_buffer_dn(:,n)']);
end


%% frame
[gate_latch_frame_dp, id_latch_frame_dp, ...
 gate_latch_frame_dn, id_latch_frame_dn, ...
 gate_latch_frame_qp, id_latch_frame_qp, ...
 gate_latch_frame_qn, id_latch_frame_qn, ...
 gate_latch_frame_buf, ...
 id] = GatesMatrix(WIDTH, HEIGHT, SCREEN_X, -1-SCREEN_Y, id);

% connection

for i = 1:HEIGHT
    ri = HEIGHT+1 - i;
    gate_fan_clk(i).AddDest([id_latch_frame_dp(ri,:), id_latch_frame_dn(ri,:)]);
    gate_fan_rstn(i).AddDest([id_latch_frame_dp(ri,:), id_latch_frame_qn(ri,:)]);
    gate_fan_refresh(i).AddDest([id_latch_frame_dp(ri,:), id_latch_frame_dn(ri,:)]);
end

for i = 1:HEIGHT
    for n = 1:WIDTH
        gate_latch_frame_buffer_qp(i,n).AddDest(id_latch_frame_dp(i,n));
        gate_latch_frame_buffer_qn(i,n).AddDest(id_latch_frame_dn(i,n));
    end
end


%% read

[gate_buffer_p_rd_row, gate_buffer_n_rd_row, id] = GatesPnBuffer(gate_input_rd_row, id);
[gate_buffer_p_rd_col, gate_buffer_n_rd_col, id] = GatesPnBuffer(gate_input_rd_col, id);
[gate_buffer_p_rd_ena, gate_buffer_n_rd_ena, id] = GatesPnBuffer(gate_input_rd_ena, id);

[gate_buffer_p_rd_row.mode] = deal(Gate.MODE_AND);
[gate_buffer_p_rd_col.mode] = deal(Gate.MODE_AND);
[gate_buffer_n_rd_row.mode] = deal(Gate.MODE_NOR);
[gate_buffer_n_rd_col.mode] = deal(Gate.MODE_NOR);

gate_buffer_p_rd_ena.AddDest([gate_buffer_p_rd_row.id]);
gate_buffer_p_rd_ena.AddDest([gate_buffer_p_rd_col.id]);
gate_buffer_n_rd_ena.AddDest([gate_buffer_n_rd_row.id]);
gate_buffer_n_rd_ena.AddDest([gate_buffer_n_rd_col.id]);


mode = Gate.MODE_AND;
orientation = Gate.FACE_BACK;
color = Gate.COLOR_DEFAULT;
x_offset = gate_latch_frame_dp(1,1).x;
y = 0;

gate_mux_rd = Gate(HEIGHT,WIDTH);
id_mux_rd = zeros(HEIGHT,WIDTH);

for i = 1:HEIGHT
    for n = 1:WIDTH
        x = n-1 + x_offset;
        z = HEIGHT - i;
        gate_mux_rd(i,n) = Gate(id, mode, orientation, x, y-SCREEN_Y, z, color);
        id_mux_rd(i,n) = id;
        id = id + 1;
        gate_latch_frame_qp(i,n).AddDest(id_mux_rd(i,n));
    end
end

row_index_bin = fliplr(decimalToBinaryVector(0:HEIGHT-1));

for i = 1:H_ADDR_WIDTH
    gate_buffer_p_rd_row(i).destination = id_mux_rd(row_index_bin(:,i)==1,:);
    gate_buffer_n_rd_row(i).destination = id_mux_rd(row_index_bin(:,i)==0,:);
end

LSB = 2;
M = 2^LSB;
col_index_bin = repelem(fliplr(decimalToBinaryVector(0:WIDTH/M-1)),M,1);

for n = 1+LSB:W_ADDR_WIDTH
    gate_buffer_p_rd_col(n).destination = id_mux_rd(:,col_index_bin(:,n-LSB)==1);
    gate_buffer_n_rd_col(n).destination = id_mux_rd(:,col_index_bin(:,n-LSB)==0);
end


%% output
mode = Gate.MODE_OR;
orientation = Gate.FACE_LEFT;
color = Gate.COLOR_OUTPUT;
x = 0;
y = -SCREEN_Y;

gates_output_dout = Gate.empty(M, 0);

for i = 1:M
    gates_output_dout(i) = Gate(id, mode, orientation, x, y, i-1, color);
    id = id + 1;
end


for i = 1:HEIGHT
    for n = 1:M
        for k = 1:WIDTH/M
            gate_mux_rd(i,n+(k-1)*M).destination = gates_output_dout(n).id;
        end
    end
end


%% gates
gates_input = [ ...
    gate_input_clk     , ...
    gate_input_rstn    , ...
    gate_input_din     , ...
    gate_input_wr_ena  , ...
    gate_input_wr_row  , ...
    gate_input_wr_col  , ...
    gate_input_refresh , ...
    gate_input_rd_row  , ...
    gate_input_rd_col  , ...
    gate_input_rd_ena    ...
];

gates_output = gates_output_dout;

gates_fan_buffer = [ ...
    gate_fan_clk,    ...
    gate_fan_rstn,   ...
    gate_fan_din_p,  ...
    gate_fan_din_n,  ...
    gate_fan_we,     ...
    gate_fan_refresh ...
];

gates_pn_buffer = [ ...
    gate_buffer_pn_wr_row, ...
    gate_buffer_pn_wr_col, ...
    gate_buffer_p_rd_row,  ...
    gate_buffer_n_rd_row,  ...
    gate_buffer_p_rd_col,  ...
    gate_buffer_n_rd_col,  ...
    gate_buffer_p_rd_ena   ...
    gate_buffer_n_rd_ena   ...
];

gates_mux = [ ...
    gate_demux_wr_row, ...
    gate_demux_wr_col, ...
    reshape(gate_mux_rd,1,[])
];

gates_latch = [ ...
    reshape(gate_latch_frame_buffer_dp,1,[]) , ...
    reshape(gate_latch_frame_buffer_dn,1,[]) , ...
    reshape(gate_latch_frame_buffer_qp,1,[]) , ...
    reshape(gate_latch_frame_buffer_qn,1,[]) , ...
    reshape(gate_latch_frame_buffer_buf,1,[]),   ...
    reshape(gate_latch_frame_dp,1,[]) , ...
    reshape(gate_latch_frame_dn,1,[]) , ...
    reshape(gate_latch_frame_qp,1,[]) , ...
    reshape(gate_latch_frame_qn,1,[]) , ...
    reshape(gate_latch_frame_buf,1,[])   ...

];

gates = [ ...
    gates_input,   ...
    gates_output,  ...
    gates_fan_buffer, ...
    gates_pn_buffer, ...
    gates_mux, ...
    gates_latch
];

if (~NetCheck(gates))
    error('创建网表失败');
end


%% write net file
fid = fopen(out_file_path, 'wb');
if fid == -1
    error('cannot create file');
end

% 文件头
INPUT = 0;
OUTPUT = 1;
num_ports = num_inputs + 1;
width = WIDTH + 1;
height = max([BUFFER_Y, SCREEN_Y]) + 6;


NetWriteGates(fid, gates);
NetWritePorthead(fid, width,height, num_ports);
NetWritePort(fid, INPUT,'I_clk'    ,gate_input_clk    );
NetWritePort(fid, INPUT,'I_clear_n',gate_input_rstn   );
NetWritePort(fid, INPUT,'I_din'    ,gate_input_din    );
NetWritePort(fid, INPUT,'I_wr_ena' ,gate_input_wr_ena );
NetWritePort(fid, INPUT,'I_wr_row' ,gate_input_wr_row );
NetWritePort(fid, INPUT,'I_wr_col' ,gate_input_wr_col );
NetWritePort(fid, INPUT,'I_refresh',gate_input_refresh);
NetWritePort(fid, INPUT,'I_rd_row' ,gate_input_rd_row );
NetWritePort(fid, INPUT,'I_rd_col' ,gate_input_rd_col );
NetWritePort(fid, INPUT,'I_rd_ena' ,gate_input_rd_ena );
NetWritePort(fid,OUTPUT,'O_dout'   ,gates_output_dout );


fclose(fid);


end