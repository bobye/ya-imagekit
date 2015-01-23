function [ ] = copy_clust( image_path, names )

reverseStr = '';
for i=1:length(names)
    names(i).label = image_label(i);
    [~, tmp, ~] = fileparts(names(i).name); [~, tmp, ~] = fileparts(tmp);
    tmp2 = ['clusters/' num2str(names(i).label) '/' tmp '.jpg'];
    tmp = [image_path names(i).author '/' tmp '.jpg'];
    copyfile(tmp,tmp2)
    % Display the progress
    percentDone = 100 * i / length(names);
    msg = sprintf('Percent done: %3.1f', percentDone); 
    fprintf([reverseStr, msg]);
    reverseStr = repmat(sprintf('\b'), 1, length(msg));     
end


end

