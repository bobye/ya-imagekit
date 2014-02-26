
addpath misc/jsonlab
addpath Laplacian

colorpairs = load('../cf-pairseg-westlake.txt');
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

L = L + .01*m/n * W;
D = diag(sum(L));


[V E] = eigs( sparse(D - L), sparse(D), 20, 'sm');
V = V(:,1:end-1)*diag(1./diag(E(1:end-1,1:end-1)));

idx = randi(n,1);
sv = V(idx,:);

d = repmat(sv, [size(V,1) 1]) - V; 
d = sqrt(sum(d.^2, 2));
d = exp( - d.^2/mean(d)^2/2);
hist(d,30);

obj.color = reshape(colordict', 1, 3*length(colordict));

%obj.v = full(L(idx,:)); 
obj.v = d';

obj.pivot = colordict(idx,:);

length(nonzeros(L(idx,:)))
mean(d)
colordict(idx,:)

savejson('', obj, 'color-themes/SharonLin-chi13/c3/examples/labcount.json');











