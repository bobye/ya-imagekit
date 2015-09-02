addpath('/gpfs/work/j/jxy198/software/mosek/7/toolbox/r2013a/');
addpath('/gpfs/work/j/jxy198/ya-imagekit/src/misc/imrender/vgg'); 
addpath('/gpfs/work/j/jxy198/ya-imagekit/src/misc'); 

global problemMap problemSet
problemMap = containers.Map('KeyType', 'char', 'ValueType', 'double');
problemSet = {};