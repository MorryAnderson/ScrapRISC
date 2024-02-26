function uuid4 = GenUUID4(file_name)
    % The UUID version 4 has the format:
    % xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    % where x is any hexadecimal digit and y is one of 8, 9, A, or B.   

    file_name = char(file_name);
    target_length = 16;
    original_length = length(file_name);

    if original_length < target_length
        num_repeats = ceil(target_length / original_length);
        extended_string = repmat(file_name, 1, num_repeats);
        extended_string = extended_string(1:target_length);
    else
        extended_string = file_name(1:target_length);
    end

    hex_str = reshape(transpose(dec2hex(extended_string)),1,[]);
    hex_str(13) = '4';
    hex_str(17) = '8';

    uuid4(1:8) = hex_str(1:8);
    uuid4(9) = '-';
    uuid4(10:13) = hex_str(9:12);
    uuid4(14) = '-';
    uuid4(15:18) = hex_str(13:16);
    uuid4(19) = '-';
    uuid4(20:23) = hex_str(17:20);
    uuid4(24) = '-';
    uuid4(25:36) = hex_str(21:32);
    
    uuid4 = lower(string(uuid4));
end

