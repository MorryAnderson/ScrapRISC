function NetWritePorthead(fid,x_len,y_len,num_ports)
    fprintf(fid, '%s', '.net');  %.net
    fwrite(fid, x_len, 'uint32');  % width
    fwrite(fid, y_len, 'uint32');  % height
    fwrite(fid, num_ports, 'uint8');  % number of ports
end

