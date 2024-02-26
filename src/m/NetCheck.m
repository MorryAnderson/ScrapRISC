function [ok] = NetCheck(gates)
    ok = 0;
    num_gates = numel(gates);    
    
    %% check id duplication
    ids = zeros(num_gates,1);
    for i=1:num_gates
        ids(i) = gates(i).id;
    end

    [unique_ids, ~, index] = unique(ids);
    counts = hist(index, unique(index)); %#ok<*HIST>
    duplicate_indices = find(counts > 1);

    if ~isempty(duplicate_indices)
        disp('Error: ID duplicated:');
        disp(unique_ids(duplicate_indices));
        return;
    else
        disp('Info: ID checked');
    end


    %% check overlapping
    pos = zeros(num_gates,3);
    for i=1:num_gates
       pos(i,:) = [gates(i).x, gates(i).y, gates(i).z];
    end

    [unique_pos, ~, idx] = unique(pos, 'rows', 'stable');
    counts = hist(idx, unique(idx));
    duplicate_indices = find(counts > 1);

    if ~isempty(duplicate_indices)
        disp('Error: overlapping:');
        disp(unique_pos(duplicate_indices, :));
        return;
    else
        disp('Info: overlapping checked');
    end
    
    
    %% check fanout
    for i=1:num_gates
       if (gates(i).num_dest > 255)
           fprintf('Error: exceed max fanout,id = %d, fan = %d\n', gates(i).id, gates(i).num_dest);
           return;
       end
    end
    disp('Info: fanout checked');
    
    
    %% check connection
    for i=1:num_gates
        gate = gates(i);
        dst = gate.destination;
        exist = ismember(dst, ids);
        if (~all(exist))
            index = find(~exist,1);
            fprintf('Error: connection error,id = %d does not exist (pointed by id = %d)\n', dst(index), gates(i).id);
            return;
        end
    end
    disp('Info: connection checked');
    
    ok = 1;
end

