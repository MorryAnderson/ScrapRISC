function GenerateIcon(str, icon_path)
    % mesh size
    str = char(str);

    if (strcmp(str(1:3),'M3_'))
        str = str(4:end);
    end

    len = length(str);
    n = ceil(sqrt(len)); 
    
    % black background
    imgSize = 128;
    img = zeros(imgSize, imgSize);

    % first n^2 char of the string
    if length(str) > n*n
        str = str(1:n*n);
    end

    % char size
    charSize = floor(imgSize / n);
    
    % char pos
    [row, col] = meshgrid(0:n-1, 0:n-1);
    x_offset = charSize / 4;
    y_offset = charSize / 2;
    col = col(:) * (imgSize / n) - x_offset;
    row = row(:) * (imgSize / n) - y_offset;

    % distribute
    for i = 1:length(str)
        charImg = insertText(img, [col(i) row(i)], str(i), 'Font', 'Consolas', 'FontSize', charSize, 'BoxOpacity', 0, 'TextColor', 'white');
        img = charImg(:, :, 1); % gray
    end
    
    % save
    imwrite(img, icon_path);
end

