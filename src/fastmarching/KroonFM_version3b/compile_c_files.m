% This script will compile all the C files of the registration methods
cd('functions');
files=dir('*.c');
clear msfm2d
mex -Dchar16_t=UINT16_T msfm2d.c
clear msfm3d
mex -Dchar16_t=UINT16_T msfm3d.c
cd('..');

cd('shortestpath');
clear rk4
mex -Dchar16_t=UINT16_T rk4.c
cd('..')
