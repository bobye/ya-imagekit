function [ ent, mu ] = eval_clust( names, featVec )
%

cluster_size = 5:5:50;
artists= unique({names.author});
label_map=zeros(length(names),1);

for i=1:length(artists)
    label_map(strcmp({names.author}, artists(i))) = i;
end

image_label=zeros(length(names), length(cluster_size));
for i=1:length(cluster_size);
    [image_label(:,i), ~] = kmeans(featVec, cluster_size(i));
end

[ent, mu] = muEnt_clust(label_map, image_label);
%Y=pdist(featVec, 'cosine');Z=linkage(Y);dendrogram(Z);
%[image_label] = cluster(Z,'maxclust',100);


end

