clear;
% a bag-of-words framework for dictionary learning and feature clustering
%% set hyper-parameters
dim = 400;
max_size = 120000;
voc_size = 200;

path='/gpfs/group/w/wang/jianbo/paintings/Westlake/';
signature_path=[path 'supgrad_v1/'];
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
    column = min(size(gradient, 2), max_size - idx + 1);
    featSet = gradient(1:(end-1),1:column)';
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
