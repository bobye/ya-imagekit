%% create synthetic examples
%clear;
addpath('../SuperPixelDisplacementInterpolation');

%% first example
height= 20;
width = 40;
%f = @(x) x;
%f = @(x) x + 0.3*(rand(size(x))-0.5);
%f = @(x) (x>0.5);
f = @(x) (x>0.5) + 0.3*(rand(size(x))-0.5);


image1 = zeros([height, width, 3]); image1(:,:,3) = 1.0;  
image1(:,:,1) = bsxfun(@(x, y) f((y-1)/(width-1)) .* ones(width,1), image1(:,:,2)', (1:width)')';
image1(:,:,2) = bsxfun(@(x, y) f((y-1)/(width-1)) .* ones(width,1), image1(:,:,3)', (1:width)')';

image_rgb = (255*mat2gray(image1(:,:,1)));
image_rgb(:,:,2) = (255*mat2gray(image1(:,:,2)));
image_rgb(:,:,3) = (255*mat2gray(image1(:,:,3)));
image_rgb = uint8(image_rgb);

%% second example
%image_rgb = imread('blog08.png');
%image_rgb = imread('Bezold Effect.jpg');
%image_rgb = imread('albers-654.jpg');
%image_rgb = imread('08color2.jpg'); image_rgb = image_rgb(181:end,:,:);
%image_rgb = imread('sample1.png');

%%
if max(size(image_rgb)) > 50
    image_rgb = imresize(image_rgb, 50 / max(size(image_rgb)));
end
height= size(image_rgb,1);
width = size(image_rgb,2);
%% calcuate signatures
profile on
gradient = SuperPixelGradv2(image_rgb, [-ones(height, width/2), ones(height, width/2)]);
profile viewer

%% display result
figure; imshow(-kron(gradient, ones(10)), []);
scale = 1;
image_rgb = im2double(image_rgb);
im(:,:,1) = kron(image_rgb(:,:,1), ones(scale));
im(:,:,2) = kron(image_rgb(:,:,2), ones(scale));
im(:,:,3) = kron(image_rgb(:,:,3), ones(scale));
figure; imshow(im, []);


