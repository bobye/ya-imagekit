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
    featSet= gradient(1:(end-1),:)';
    featWeight=gradient(end,:)';
    for j=1:voc_size
        featVec(i,j) = sum(exp( - sum(abs(bsxfun(@minus, featSet, C(j,:))), 2) ...
            / (1.*avg_dist(j))).*featWeight)/ size(gradient, 2) ;
    end
    
    %
    % Display the progress
    percentDone = 100 * i / length(names);
    msg = sprintf('Percent done: %3.1f', percentDone); 
    fprintf([reverseStr, msg]);
    reverseStr = repmat(sprintf('\b'), 1, length(msg));      
end

featVec(:,any(isnan(featVec))) = [];
%% clustering
disp 'clustering images ...'
[ent, mu] = eval_clust(names, featVec);
plot(ent, mu,'k*-'); hold on
ylim([0, 0.65]);
xlim([2, 7]);
xlabel('Entropy of clusters');
ylabel('NMI');
%% copy image files
%copy_clust(image_path, names);