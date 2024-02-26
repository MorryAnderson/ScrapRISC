function [gate_p, gate_n, id_nxt] = GatesPnBuffer(gate_in, id)

    orientation = Gate.FACE_UP;
    color = Gate.COLOR_DEFAULT;

    x = gate_in(1).x;
    y = gate_in(1).y;

    width = numel(gate_in);
    gate_p = Gate.empty(width, 0);
    gate_n = Gate.empty(width, 0);

    for i = 1:width
        gate_in(i).destination = [id, id+1];
        
        gate_p(i) = Gate(id, Gate.MODE_OR, orientation, x+1, y, i-1, color);
        id = id + 1;
        gate_n(i) = Gate(id, Gate.MODE_NOR, orientation, x+2, y, i-1, color);
        id = id + 1;
    end

    id_nxt = id;

end