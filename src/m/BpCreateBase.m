function BpCreateBase(fid,bid,color,width,heigth,comma)
    if (nargin < 7)
        comma = 0;
    end
    fprintf(fid, '{\n');
    fprintf(fid, '\t\"bounds\":{\"x\":%d, \"y\":%d, \"z\":%d},\n',width,heigth,1);
    fprintf(fid, '\t\"pos\":{\"x\":0, \"y\":%d, \"z\":-1},\n',-heigth+1);    
    fprintf(fid, '\t\"shapeId\":\"%s\",\n',bid);
    fprintf(fid, '\t\"xaxis\":1,\n');
    fprintf(fid, '\t\"zaxis\":3,\n');
    fprintf(fid, '\t\"color\":\"%s\"\n',color);
    fprintf(fid, '}');
    if (comma)
        fprintf(fid, ',');
    end
    fprintf(fid, '\n');
end

