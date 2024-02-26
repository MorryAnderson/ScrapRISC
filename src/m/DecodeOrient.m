function [xaxis,zaxis,x,y,z] = DecodeOrient(gate)
    lsb = int8(bitand(gate.orientation, 0x0F));
    msb = int8(bitand(bitshift(gate.orientation, -4), 0xF));
    t1 = msb - 2^4 * bitget(msb, 4);
    t2 = lsb - 2^4 * bitget(lsb, 4);
    t1 = t1 + Gate.TABLE_CENTER;
    t2 = t2 + Gate.TABLE_CENTER;
    x = gate.x - Gate.TABLE_OFFSET(t1,t2,1);
    y = gate.y - Gate.TABLE_OFFSET(t1,t2,2);
    z = gate.z - Gate.TABLE_OFFSET(t1,t2,3);    
    xaxis = Gate.TABLE_XZ(t1,t2,1);
    zaxis = Gate.TABLE_XZ(t1,t2,2);
end

