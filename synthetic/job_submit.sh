#PBS -l nodes=1:ppn=1
#PBS -l walltime=1:30:00
#PBS -l pmem=2gb
cd $PBS_O_WORKDIR
module load matlab/R2013a
matlab -nosplash -nojvm -r "genSignature('$file');exit;"
