function [gate_fan, id_nxt] = GatesFanBuffer(gate_in,width, id)

    mode = Gate.MODE_OR;
    orientation = Gate.FACE_LEFT;
    color = Gate.COLOR_DEFAULT;
    
    x = gate_in.x + 1;
    y = gate_in.y;

    gate_fan  = Gate.empty(width,0);
    id_fan = zeros(1,width);
    
    for i = 1:width
        gate_fan(i) = Gate(id, mode, orientation, x, y, i-1, color);
        id_fan(i) = id;
        id = id + 1;
    end
    gate_in.AddDest(id_fan);

    id_nxt = id;

end