% fast marching method lib
addpath('src/fastmarching/KroonFM_version3b/functions/');

k = 0;
filepath='subseg_map/';
name=[filepath, 'bseg_map-', num2str(k)];

while (exist([name, '.dat'], 'file'))

% load subregion of segmentation map
% 1 and -1 are two segments, 0 is other pixels
L=load([name, '.dat']); 

F = ones(size(L));
[I, J]= find(L == 1);
T1 = msfm2d(F, [I,J]', true, true);

F = ones(size(L));
[I, J]= find(L == -1);
T2 = msfm2d(F, [I,J]', true, true);

T1 = T1-0.5; T1(T1<0) = 0;
T2 = T2-0.5; T2(T2<0) = 0;
T = T2 - T1;

dlmwrite([name, '-d.dat'], T, 'delimiter', ' ');
k = k + 1;
name=[filepath, 'bseg_map-', num2str(k)];
end