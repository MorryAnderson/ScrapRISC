function NetWritePort(fid,is_output,name,gates)
    
    if (is_output)
        dir = 'O';
    else
        dir = 'I';
    end
    
    width = numel(gates);
    
    fprintf(fid, '%s', dir);  % Input('I')/ Output('O')
    fwrite(fid, width,'uint8');  % port width
    fwrite(fid, strlength(name), 'uint8'); % length of port name (exclude '\0')
    fprintf(fid, '%s\0', name);  % port name, ending with '\0'

    % output port connection
    for i=1:width
        fwrite(fid,1,'uint8');  % how many gates does this bit of port is connected to
        fwrite(fid,gates(i).id,'uint16');  %  id of gate
    end
    fwrite(fid,0xFFFF,'uint16');  % frame tail
end

