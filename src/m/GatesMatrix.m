function [gate_dp, id_dp, gate_dn, id_dn, gate_qp, id_qp, gate_qn, id_qn, gate_buf, id_nxt] = ...
    GatesMatrix(width, height, x_offset, y_offset, id)

gate_dp = Gate(height,width);
gate_dn = Gate(height,width);
gate_qp = Gate(height,width);
gate_qn = Gate(height,width);
gate_buf = Gate(height,width);

data_id = reshape((1:height*width*5)+id,height,width,[]);

id_dp = data_id(:,:,1);
id_dn = data_id(:,:,2);
id_qp = data_id(:,:,3);
id_qn = data_id(:,:,4);
id_buf = data_id(:,:,5);
id_nxt = data_id(end,end,end)+1;


orientation = Gate.FACE_FRONT;
color = Gate.COLOR_BLACK;


for i = 1:height
    for n = 1:width
        %x = width-1 - (n-1);
        %z = i-1;
        x = n-1;
        z = height-1 - (i-1);
        gate_dp(i,n)  = Gate(id_dp(i,n),  Gate.MODE_NAND, orientation, x_offset + x, y_offset - 0, z, color, id_qp(i,n));  
        gate_dn(i,n)  = Gate(id_dn(i,n),  Gate.MODE_NAND, orientation, x_offset + x, y_offset - 1, z, color, id_qn(i,n)); 
        gate_qn(i,n)  = Gate(id_qn(i,n),  Gate.MODE_NAND, orientation, x_offset + x, y_offset - 2, z, color, id_buf(i,n)); 
        gate_buf(i,n) = Gate(id_buf(i,n), Gate.MODE_AND,  orientation, x_offset + x, y_offset - 3, z, color, id_qp(i,n));
        gate_qp(i,n)  = Gate(id_qp(i,n),  Gate.MODE_NAND, orientation, x_offset + x, y_offset - 4, z, color, id_qn(i,n));  
    end
end


end