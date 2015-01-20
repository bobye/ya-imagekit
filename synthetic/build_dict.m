clear;
%% set hyper-parameters
dim = 400;
max_size = 100000;
voc_size = 200;

%%
path='/gpfs/group/w/wang/jianbo/paintings/westlake2-sig256/';

names = dir(fullfile(path , '*.supgrad.mat' ));

disp 'load visual features ... '
features = zeros(max_size, dim);
idx=1;
for i=1:length(names)
    load([path names(i).name]);
    column = min(size(gradient, 3), max_size - idx + 1);
    featSet = reshape(gradient(:,:,1:column), dim, column)';
    features(idx:idx+column-1, :) = featSet;
    idx = idx + column;
    if (idx > max_size) 
        break;
    end
end

%%
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
for i=1:length(names)
    load([path names(i).name]);
    featSet = reshape(gradient, dim, size(gradient, 3))';
    for j=1:voc_size
        featVec(i,j) = sum(exp( - sum(bsxfun(@minus, featSet, C(j,:)).^2, 2) ...
            / (2*avg_dist(j).^2)))/ size(gradient, 3);
    end
end

%%
disp 'clustering images ...'
image_path='/gpfs/group/w/wang/jianbo/paintings/westlake2/';
[image_label, image_center] = kmeans(featVec, 10);
for i=1:length(names)
    names(i).label = image_label(i);
    [~, tmp, ~] = fileparts(names(i).name);
    [~, tmp, ~] = fileparts(tmp);
    tmp2 = ['clusters/' num2str(names(i).label) '/' tmp '.jpg'];
    tmp = [image_path tmp '.jpg'];
    copyfile(tmp,tmp2)
end