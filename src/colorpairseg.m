clear
addpath misc/jsonlab
addpath Laplacian

colorpairs = load('../cf-pairseg.txt');
%jsondata = loadjson('color-themes/SharonLin-chi13/c3/data/xkcd/c3_data.json');
%colorbins = reshape(jsondata.color, [3, length(jsondata.color)/3])';

colorpairs = 5*floor(colorpairs/5); % discretization
color1= colorpairs(:,1:3);
color2= colorpairs(:,4:6);
[colordict, ~, ic] = unique([color1;color2], 'rows');
pairs = reshape(ic, [size(color1,1) 2]);

n = size(colordict,1);
m = size(pairs,1);
L = sparse(n,n);
for i=1:m
    L(pairs(i,1), pairs(i,2)) = L(pairs(i,1),pairs(i,2)) + 1;
end
L = L + L'; 
A = adjacency(colordict, 'nn', 6);
t = 2 * mean(sum(A)/6)^2;

[A_i, A_j, A_v] = find(A);

W=A;
for i = 1: size(A_i)  
  % replece distances by 1
  % gaussain kernel can be used instead of 1:
  W(A_i(i), A_j(i)) = exp(-A_v(i)^2/t);
  %W(A_i(i), A_j(i)) = 1;
end;

%L0 = repmat(full(sum(L))/n, [n, 1]);
D = sum(L);
L = diag(sqrt(1./D)) * L * diag(sqrt(1./D));
L =  L + 0.05 * W;
L0 = W;
D = diag(sum(L));
D0 = diag(sum(L0));

[V E] = eigs( sparse(D - L), sparse(D), 100, 'sm');
[V0 E0] =eigs(sparse(D0 - L0), sparse(D), 100, 'sm');

V = V(:,2:end-1)*diag(1./diag(E(2:end-1,2:end-1).^(7/2)));
V0 = V0(:,2:end-1)*diag(1./diag(E0(2:end-1,2:end-1).^(3/2)));

%% 
[idx d] = gettheme(1, ones(n,1), V, V0, [3670]);
%[idx d] = gettheme(1, ones(n,1), V, V0);

if (length(idx) ~= 1) %reranking
    [Y, eigs] = cmdscale(pdist(V(idx,:),'euclidean'));
    [~, idxd] = sort(Y(:,1));
    idx = idx(idxd);
else
    hist(d,30);
end

obj.color = reshape(colordict', 1, 3*length(colordict));
%obj.v = full(V(:,end-5)-min(V(:,end-5))); 
%obj.v = full(-V(:,end-5)+max(V(:,end-5))); 
obj.v = (d)';
obj.pivot = idx-1; %number from 0

%length(nonzeros(L(idx,:)))
obj.pscore = 100*mean(d).^(1/5);
%colordict(idx,:)

savejson('', obj, 'color-themes/SharonLin-chi13/c3/examples/labcount.json');
!open http://localhost:8001/examples/colors.html











