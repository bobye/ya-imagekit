function [ sig ] = superPixelGrad( Image, Label )
% superpixel gradient for synthetic examples
% I -  images
% L -  labels: 0 irrelevant, 1 first segment, -1 second segment

width = size(Image,1);
height = size(Image,2);

AllPoints = reshape(Image, width*height, 3);

%% solve relative distance
disp 'compute relative spatial coordinates ... '
% load fast marching method lib
addpath('../src/fastmarching/KroonFM_version3b/functions/');

% compute nearest distance to first segment
F = ones(size(Label));
[I, J]= find(Label == 1);
T1 = msfm2d(F, [I,J]', true, true);
IDX = sub2ind([width, height], I, J);
P1 = AllPoints(IDX, :);
[P1, ~, k] = unique(P1, 'rows');
C1 = histc(k, 1:size(P1,1));
P1 = double(P1);

% compute nearest distance to second segment
F = ones(size(Label));
[I, J]= find(Label == -1);
T2 = msfm2d(F, [I,J]', true, true);
IDX = sub2ind([width, height], I, J);
P2 = AllPoints(IDX, :);
[P2, ~, k] = unique(P2, 'rows');
C2 = histc(k, 1:size(P2,1));
P2 = double(P2);

% final relative distance
T1 = T1-0.5; T1(T1<0) = 0;
T2 = T2-0.5; T2(T2<0) = 0;
D = T2 - T1;
D = D(:);
Dmax = max(D);
Dmin = min(D);

disp '[done]'
%% build transportation
disp 'solve transportation problem ... '
n = size(P1,1);
m = size(P2,1);

% ---build-in linprog ---
%f = pdist2(P1,P2,'sqeuclidean');
%A = zeros(n+m, n*m);
%for k=1:n
%    A(k, k:n:(n*m)) = 1;
%end
%for k=1:m
%    A(n+k, (k-1)*n + (1:n)) = 1;
%end
%
%A = sparse(A);
%tic;
%options = optimoptions('linprog', 'Algorithm', 'simplex');
%[w, ~] = linprog(f(:), [], [], A, [C1'/sum(C1), C2'/sum(C2)], zeros(n*m,1), [], []);
%toc;

% --- Mosek solver ---
addpath('/gpfs/work/j/jxy198/software/mosek/7/toolbox/r2013a/');
prob.c = pdist2(P1,P2,'sqeuclidean'); prob.c = prob.c(:);
subi = [repmat(1:n, 1, m), reshape(repmat((1:m) + n, n, 1), 1, n*m)];
subj = [1:(n*m), 1:(n*m)];
valij = ones(1,2*n*m);
prob.a = sparse(subi, subj, valij);
prob.blc = [C1/sum(C1); C2/sum(C2)];
prob.buc = prob.blc;
prob.blx = sparse(n*m, 1);
prob.bux = [];
param.MSK_IPAR_OPTIMIZER = 'MSK_OPTIMIZER_PRIMAL_SIMPLEX'; 
[~, w] = mosekopt('minimize echo(0)', prob, param);

w = w.sol.bas.xx;
w(w<1E-10) = 0;
w = sparse(reshape(w, n, m));

disp '[done]';

%% build signature
disp 'build up histogram ... '
sizeT = 20;
sizeD = 20;
h = 3.;
sig = zeros(sizeT, sizeD);
for i=1:sizeT
    [I, J] = find(w);
    P = P1(I,:) * (sizeT-i)/(sizeT-1) + P2(J,:) * (i-1)/(sizeT-1); % interpolated distribution
    W = w(w>0); % weights of interpolated points
    for j=1:sizeD
        IDX = floor((D - Dmin)*sizeD /(Dmax - Dmin + 1E-2)) == (j-1);
        [Q, ~, k] = unique(AllPoints(IDX(:),:), 'rows');
        C = histc(k, 1:size(Q,1));
        Q = double(Q);
        if (~isempty(Q))
            tmp = sum(...
                bsxfun(@times, ...
                exp( - pdist2(P, Q, 'sqeuclidean') / (2*h*h)), W));
            sig(i, j) = tmp * C / sum(C);
        end
    end
end

disp '[done]'

end
