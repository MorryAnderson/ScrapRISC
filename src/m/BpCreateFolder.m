function fid = BpCreateFolder(file_name,out_folder_path)

    file_name = string(file_name);
    out_folder_path = string(out_folder_path);

    %% create folder
    uuid4 = GenUUID4(file_name);
    folder_path  = out_folder_path + '/' + uuid4;

    % check existence
    if exist(folder_path, 'dir') == 0
        status = mkdir(folder_path);
        if status ~= 1
            error(['cannot create folder: ' folder_path]);
        end
        fprintf('Info: folder created: \"%s\"\n',folder_path);
    else
        fprintf('Warning: folder overwritten \"%s\"\n',folder_path);
    end

    %% create icon
    GenerateIcon(file_name,folder_path + '/icon.png');

    %% description.json
    fid = fopen(folder_path + '/description.json', 'wt');
    if fid == -1
        error('cannot create file description.json');
    end

    fprintf(fid, '{\n');
    fprintf(fid, '\t\"description\" : \"%s\",\n', file_name);
    fprintf(fid, '\t\"localId\" : \"%s\",\n', uuid4);
    fprintf(fid, '\t\"name\" : \"%s\",\n', file_name);
    fprintf(fid, '\t\"type\" : \"Blueprint\",\n');
    fprintf(fid, '\t\"version\" : 0\n');
    fprintf(fid, '}\n');

    fclose(fid);

    %% blueprint.json
    fid = fopen(folder_path + '/blueprint.json', 'wt');
    if fid == -1
        error('cannot create file blueprint.json');
    end

end

