function [ sig ] = SuperPixelGradv2( Image, Label )
% superpixel gradient for synthetic examples
% Image -  images (row-major)
% Label -  labels: 0 irrelevant, 1 first segment, -1 second segment

width = size(Image,1);
height = size(Image,2);

AllColors = reshape(Image, width*height, 3);
[Y, X] = meshgrid(1:height, 1:width);
AllPositions = [X(:), Y(:)];

% preprocess into two color and spatial histograms
[I, J]= find(Label == 1);
IDX = sub2ind([width, height], I, J);
P1 = AllColors(IDX, :);
Q1 = AllPositions(IDX, :);
[P1, ~, k] = unique(P1, 'rows');
C1 = histc(k, 1:size(P1,1));
P1 = double(P1);

[I, J]= find(Label == -1);
IDX = sub2ind([width, height], I, J);
P2 = AllColors(IDX, :);
Q2 = AllPositions(IDX, :);
[P2, ~, k] = unique(P2, 'rows');
C2 = histc(k, 1:size(P2,1));
P2 = double(P2);


%% build transportation

% --- Ported Solver ---
[rcode1, ~, Pw, ~] = OptimalTransport(P1, P2, C1, C2);
[rcode2, ~, Qw, ~] = OptimalTransport(Q1, Q2, ones(size(Q1,1),1), ones(size(Q2,1),1));

if (rcode1 ~= 0 || rcode2 ~= 0)
    return;
end

%% build signature
%disp 'build up histogram ... '
sizeP = 20;
sizeQ = 20;
h = 3.;
sig = zeros(sizeP, sizeQ);


WP = Pw(Pw>0); WP = full(WP(:)); % weights of interpolated points
[I, J] = find(Pw);
P1vec = P1(I,:); P1vec=P1vec(:);
P2vec = P2(J,:); P2vec=P2vec(:);
P = [P1vec, P2vec] * [(sizeP-1):-1:0; 0:(sizeP-1)] / (sizeP-1);
P = reshape(P, [length(I), 3, sizeP]);

WQ = Qw(Qw>0); WQ = full(WQ(:));
[I, J] = find(Qw); len = length(I);
Q1vec = Q1(I,:); Q1vec=Q1vec(:);
Q2vec = Q2(J,:); Q2vec=Q2vec(:);
Qtmp = [Q1vec, Q2vec] * [(sizeQ-1):-1:0; 0:(sizeQ-1)] / (sizeQ-1);
Qtmp = reshape(Qtmp, [len, 2, sizeQ]);
Image=double(Image);

Q = cell(sizeQ,1);
WQi = cell(sizeQ,1);
for i=1:sizeQ
    Q{i} = interp2color(Qtmp(:,:,i));
    Q{i} = reshape(round(Q{i}), len, 3);
    [Q{i}, ~, k] = unique(Q{i}, 'rows');
    [~, ind] = histc(k, 1:size(Q{i},1));
    WQi{i} = accumarray(ind, WQ);
end


PP = cell(sizeP,1);
for i=1:sizeP
    PP{i} = kde(P(:,:,i)', h, WP');
end
QQ = cell(sizeQ,1);
for i=1:sizeQ
    QQ{i} = kde(Q{i}', h, WQi{i}');
end

for i=1:sizeP
    for j=1:sizeQ
        sig(i,j) = sqrtDistance(PP{i}, QQ{j});
    end
end

sig = pi/2 - acos(sig / max(sig(:)));

function color = interp2color(x)
  color=  [vgg_interp2(Image(:,:,1), x(:,2), x(:,1)); ...
           vgg_interp2(Image(:,:,2), x(:,2), x(:,1)); ...
           vgg_interp2(Image(:,:,3), x(:,2), x(:,1))];
end

function D = sqrtDistance(p1, p2)
    L1 = evaluate(p1, p2);
    L2 = evaluate(p2, p2);
    W = getWeights(p2);
    D = (sqrt(L1).*sqrt(L2) * W') / (L2 * W');
end

end