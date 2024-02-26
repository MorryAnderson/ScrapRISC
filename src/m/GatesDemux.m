function [gate_pn, gate_demux, id_nxt, x_offset] = GatesDemux(gate_in, id, reversed, gate_ena)

    orientation = Gate.FACE_UP;
    color = Gate.COLOR_DEFAULT;
    
    x = gate_in(1).x;
    y = gate_in(1).y;
    
    width = numel(gate_in);
    depth = 2^width;

    gate_p = Gate.empty(width, 0);
    gate_n = Gate.empty(width, 0);

    for i = 1:width
        gate_in(i).destination = [id, id+1];
        
        gate_p(i) = Gate(id, Gate.MODE_OR, orientation, x+1, y, i-1, color);
        id = id + 1;
        gate_n(i) = Gate(id, Gate.MODE_NOR, orientation, x+2, y, i-1, color);
        id = id + 1;
    end
    gate_pn = [gate_p, gate_n];

    
    orientation = Gate.FACE_RIGHT;
    color = Gate.COLOR_WHITE;
    x = gate_in(1).x + 3;
    y = gate_in(1).y;
    
    index_bin = fliplr(decimalToBinaryVector(0:depth-1));
    
    gate_demux = Gate.empty(depth, 0);
    id_mux = zeros(depth,1);
    
    for i = 1:depth
        if (reversed)
            z = depth-1 - (i-1);
        else
            z = i-1;
        end
        gate_demux(i) = Gate(id, Gate.MODE_AND, orientation, x, y, z, color);
        id_mux(i) = id;
        id = id + 1;
    end
    
    for i = 1:width
        gate_p(i).destination = id_mux(index_bin(:,i)==1);
        gate_n(i).destination = id_mux(index_bin(:,i)==0);
    end
    
    if (nargin >= 4)
        gate_ena.AddDest(id_mux);
    end
    x_offset = gate_demux(1).x + 1;
    
    id_nxt = id;

end