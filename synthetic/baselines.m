clear;
path='/gpfs/group/w/wang/jianbo/paintings/Westlake/';
image_path=[path 'westlake_by_artists/'];

%% load set of images

names = struct([]);
list_dir = dir(fullfile(image_path , '*' ));
for i=1:length(list_dir)
    list_files = dir([image_path list_dir(i).name '/*.jpg']);
    for j=1:length(list_files)
        list_files(j).author = list_dir(i).name;
    end
    if isempty(names) 
        names = list_files;
    else
        names = [names; list_files];
    end
end

%% entropy

featVec = zeros(length(names),1);
for i=1:length(names)
    im = imread([image_path names(i).author '/' names(i).name]);
    im = imresize(im, min(256 / sqrt(size(im,1) * size(im,2)), 1.0));
    im = rgb2gray(im);
    featVec(i) = entropy(im);
end
[ent, mu] = eval_clust(names, featVec);
plot(ent, mu,'r.-'); hold on
ylim([0, 0.65]);
xlim([2, 7]);
xlabel('Entropy of clusters');
ylabel('NMI');
%% Lab histogram
addpath('/gpfs/home/jxy198/work/ya-imagekit/synthetic/vislab/matlab/lab_histogram');
for i=1:length(names)
    image_filenames{i} = [image_path names(i).author '/' names(i).name];
end
featVec = lab_hist(image_filenames, '');
[ent, mu] = eval_clust(names, featVec);
plot(ent, mu,'g*-'); hold on
%% GIST
gist_path=[path 'gist/'];
featVec = zeros(length(names), 960);
for i=1:length(names)
    [~, tmp, ~] = fileparts(names(i).name);
    featVec(i,:) = load([gist_path names(i).author '/' tmp '.gist.mat'], '-ASCII');
end
[ent, mu] = eval_clust(names, featVec);
plot(ent, mu,'m+-'); hold on


%%

legend('SuperGrad', 'Entropy','Lab Hist','GIST');
