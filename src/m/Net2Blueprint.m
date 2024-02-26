function Net2Blueprint(in_file_path, out_folder_path)

%% constant
BASE_SHAPE_ID = '628b2d61-5ceb-43e9-8334-a4135566df7a';
BASE_COLOR = '222222';


%% read netlist
fid = fopen(in_file_path, 'rb');
if fid == -1
    error('cannot open file');
end


%% array segment
head = fread(fid, [1,4], '*char');

if (isequal(head,'.net'))
    num_gates = fread(fid, 1, 'uint32','ieee-le');
    gates = Gate.empty(num_gates,0);
    i = 1;
    while ~feof(fid)
        gates(i).id = fread(fid, 1, 'uint16','ieee-le');
        gates(i).mode = fread(fid, 1, 'uint8','ieee-le');
        gates(i).orientation = fread(fid, 1, 'uint8','ieee-le');
        gates(i).x = fread(fid, 1, 'int32','ieee-le');
        gates(i).y = fread(fid, 1, 'int32','ieee-le');
        gates(i).z = fread(fid, 1, 'int32','ieee-le');
        gates(i).color = fread(fid, 1, 'uint32','ieee-le');
        num_dest = fread(fid, 1, 'uint8','ieee-le');
        gates(i).destination = fread(fid, num_dest, 'uint16','ieee-le');
        eog = fread(fid, 1, 'uint16','ieee-le');
        if (eog ~= 0xFFFF)
            error('Error: wrong file format, no frame tail');
        end
        if (i >= num_gates)
            break;
        else
            i = i + 1;
        end
    end
    if (i < num_gates) 
        disp('Warning: wrong num of gates');
    end
else
    error('Error: wrong file format，no .net head in array segment');
end


%% port segment
head = fread(fid, [1,4], '*char');

if (isequal(head,'.net'))
    x_len = fread(fid, 1, 'uint32','ieee-le');
    y_len = fread(fid, 1, 'uint32','ieee-le');
else
   error('Error: wrong file format，no .net head in port segment');
end

fprintf('Info: netlist \"%s\" is read，total %d gates\n',in_file_path,i);

fclose(fid);

%% create bluprint file
[~, file_name, ~] = fileparts(in_file_path);
fid = BpCreateFolder(file_name,out_folder_path);
BpWriteFileHead(fid);

for i=1:num_gates
    BpCreateGate(fid,gates(i));
end

BpCreateBase(fid, BASE_SHAPE_ID, BASE_COLOR, x_len, y_len);
BpWriteFileTail(fid);

fclose(fid);

disp('Info: blueprint generated');

end