function NetWriteGates(fid,gates)
    fprintf(fid, '%s', '.net');  %.net
    fwrite(fid, numel(gates), 'uint32'); 

    for i=1:numel(gates)
        gate = gates(i);
        fwrite(fid,gate.id,'uint16');
        fwrite(fid,gate.mode,'uint8'); 
        fwrite(fid,gate.orientation,'uint8');
        fwrite(fid,gate.x,'int32');
        fwrite(fid,gate.y,'int32');
        fwrite(fid,gate.z,'int32');
        fwrite(fid,gate.color,'uint32');
        fwrite(fid,gate.num_dest,'uint8');
        if (gate.num_dest > 0)
           for n=1:gate.num_dest
               fwrite(fid,gate.destination(n),'uint16');
           end
        end
        fwrite(fid,0xFFFF,'uint16');  % ?¦Â
    end

end

