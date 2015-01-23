function [ent, mu] = muEnt_clust(label_map, image_cluster)
    assert(length(label_map) == size(image_cluster,1));
    n = size(image_cluster, 2);
    m = length(label_map);
    label_ent = entropyfunc(histc(label_map, 1:max(label_map))/m);
    mu = zeros(n,1);
    ent = zeros(n,1);
    for i=1:n
        ent(i) = entropyfunc(histc(image_cluster(:,i), 1:max(image_cluster(:,i))) / m);
        [~, ~, ic] = unique([label_map, image_cluster(:,i)], 'rows');
        mu(i) = (ent(i) + label_ent - entropyfunc(histc(ic, 1:max(ic)) / m))/(ent(i));       
    end
    %plot(ent, mu);
    %ylim([0, label_ent]);
end

function ent = entropyfunc(p)
    p(p==0) = [];
    ent = -sum(p.*log2(p));
end