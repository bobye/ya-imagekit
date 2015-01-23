clear;
% a bag-of-words framework for dictionary learning and feature clustering
%% set hyper-parameters
dim = 400;
max_size = 120000;
voc_size = 200;

path='/gpfs/group/w/wang/jianbo/paintings/Westlake/';
signature_path=[path 'signature/'];
image_path=[path 'westlake_by_artists/'];

%% load set of images

names = struct([]);
list_dir = dir(fullfile(signature_path , '*' ));
for i=1:length(list_dir)
    list_files = dir([signature_path list_dir(i).name '/*.supgrad.mat']);
    for j=1:length(list_files)
        list_files(j).author = list_dir(i).name;
    end
    if isempty(names) 
        names = list_files;
    else
        names = [names; list_files];
    end
end

%% load visual words
disp 'load visual words ... '
features = zeros(max_size, dim);
idx=1;
for i=1:length(names)
    load([signature_path names(i).author '/' names(i).name]);
    column = min(size(gradient, 3), max_size - idx + 1);
    featSet = reshape(gradient(:,:,1:column), dim, column)';
    features(idx:idx+column-1, :) = featSet;
    idx = idx + column;
    if (idx > max_size) 
        break;
    end
end

%% kmeans
disp 'create vocabulary ... '
tic;[idx,C] = kmeans(features,voc_size, 'Display', 'iter');toc;

save voc.mat features idx C

%%
display_network(-C', true, false);

%%
disp 'bag of words ...'
load voc.mat

avg_dist = zeros(voc_size, 1);
for i=1:voc_size
    avg_dist(i) = mean(sqrt(sum(bsxfun(@minus, features(idx == i, :), C(i,:)).^2, 2)));
end

featVec = zeros(length(names), voc_size);
reverseStr = '';
for i=1:length(names)
    load([signature_path names(i).author '/' names(i).name]);
    featSet = reshape(gradient, dim, size(gradient, 3))';
    for j=1:voc_size
        featVec(i,j) = sum(exp( - sum(bsxfun(@minus, featSet, C(j,:)).^2, 2) ...
            / (2*avg_dist(j).^2)))/ size(gradient, 3);
    end
    
    %
    % Display the progress
    percentDone = 100 * i / length(names);
    msg = sprintf('Percent done: %3.1f', percentDone); 
    fprintf([reverseStr, msg]);
    reverseStr = repmat(sprintf('\b'), 1, length(msg));      
end

%% clustering
disp 'clustering images ...'

[ent, mu] = eval_clust(names, featVec);
%% copy image files
copy_clust(image_path, names);