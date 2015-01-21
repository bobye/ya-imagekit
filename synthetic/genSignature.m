function [] = genSignature(filename)

[pathstr, name, ~] = fileparts(filename);
addpath(genpath('../src/misc/MatlabFns/'));

%% Load image

im = imread(filename);
im = imresize(im, min(256 / sqrt(size(im,1) * size(im,2)), 1.0));

disp(filename);

%% Compute superpixels
tic;
disp 'compute superpixels ... '
[l, Am, Sp, d] = slic(im, 150, 30, 1., 'median');
%show(drawregionboundaries(l, im, [255 255 255]));
disp '[done]'
toc;

%% create candidate pairs 
[I, J] = find(Am);
isChosen = arrayfun(@(i) Sp(i).c, I) < arrayfun(@(i) Sp(i).c, J);
I = I(isChosen);
J = J(isChosen);
n = length(I);

%% compute signatures
disp 'start computing signatures ...'
gradient = zeros(20,20,n);
im = lab2uint8(rgb2lab(im));
reverseStr = '';
tic;
for i=1:n
    left = I(i); right = J(i);
    [I1, J1] = find(l == left | l == right);
    i_min = min(I1); i_max = max(I1);
    j_min = min(J1); j_max = max(J1);
    ROI = im(i_min:i_max, j_min:j_max, :);
    Label = l(i_min:i_max, j_min:j_max);
    leftI = Label == left;
    rightI= Label == right;
    Label(leftI) = -1;
    Label(rightI) = 1;
    Label(~(leftI | rightI)) = 0;
    gradient(:,:,i) = superPixelGrad(ROI, Label);
    %show(drawregionboundaries(Label, ROI, [255 255 255]));
    %figure;imshow(-kron(gradient(:,:,i), ones(10)), []);
    %pause;
   
    % Display the progress
    percentDone = 100 * i / n;
    msg = sprintf('Percent done: %3.1f', percentDone); %Don't forget this semicolon
    fprintf([reverseStr, msg]);
    reverseStr = repmat(sprintf('\b'), 1, length(msg));    
end
disp '[done]'
toc;

%% save results
save([pathstr '/../westlake2-sig256_new/' name '.supgrad.mat'], 'gradient');
