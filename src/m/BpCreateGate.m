function BpCreateGate(fid,gate)
    [xaxis,zaxis,x,y,z] = DecodeOrient(gate);    
    
    fprintf(fid, '{\n');
    fprintf(fid, '\t\"shapeId\":\"%s\",\n',Gate.SHAPE_ID);
    fprintf(fid, '\t\"color\":\"%s\",\n',dec2hex(gate.color,6));
    fprintf(fid, '\t\"xaxis\":%d,\n',xaxis);
    fprintf(fid, '\t\"zaxis\":%d,\n',zaxis);
    fprintf(fid, '\t\"pos\":{\"x\":%d, \"y\":%d, \"z\":%d},\n',x,y,z);
    fprintf(fid, '\t\"controller\":{\n');
    fprintf(fid, '\t\t\"active\":false,\n');
    fprintf(fid, '\t\t\"joints\":null,\n');
    fprintf(fid, '\t\t\"mode\":%d,\n',gate.mode);
    fprintf(fid, '\t\t\"id\":%d,\n',gate.id);
    fprintf(fid, '\t\t\"controllers\":[\n');
    if (gate.num_dest > 0)
        fprintf(fid, '\t\t\t{\"id\":%d}',gate.destination(1));
        if (gate.num_dest >= 2)
            for n=2:gate.num_dest
                fprintf(fid, ',\n\t\t\t{\"id\":%d}',gate.destination(n));
            end            
        end
        fprintf(fid, '\n');
    end
    fprintf(fid, '\t\t]\n');
    fprintf(fid, '\t}\n');
    fprintf(fid, '},\n');
end

