function BpCreateBlock(fid,bid,color,x,y,z,comma)
    if (nargin < 7)
        comma = 1;
    end
    fprintf(fid, '{\n');
    fprintf(fid, '\t\"pos\":{\"x\":%d, \"y\":%d, \"z\":%d},\n',x,y,z);
    fprintf(fid, '\t\"shapeId\":\"%s\",\n',bid);
    fprintf(fid, '\t\"xaxis\":1,\n');
    fprintf(fid, '\t\"zaxis\":3,\n');
    fprintf(fid, '\t\"bounds\":{\"x\":1, \"y\":1, \"z\":1},\n');
    fprintf(fid, '\t\"color\":\"%s\"\n',color);
    fprintf(fid, '}');
    if (comma)
        fprintf(fid, ',');
    end
    fprintf(fid, '\n');
end

