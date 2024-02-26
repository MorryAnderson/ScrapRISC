function [gates, id_nxt, y_nxt] = GatesInput(id, x, y, width)

    mode = Gate.MODE_OR;
    orientation = Gate.FACE_LEFT;
    color = Gate.COLOR_INPUT;

    gates = Gate.empty(width, 0);
    for i = 1:width
        gates(i) = Gate(id, mode, orientation, x, y, i-1, color);
        id = id + 1;
    end
    id_nxt = id;
    y_nxt = y - 1;

end