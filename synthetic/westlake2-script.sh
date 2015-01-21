path=/gpfs/home/jxy198/group/jianbo/paintings/westlake2
for filename in `ls -d -1 $path/*.jpg`
do
##filename=/gpfs/home/jxy198/group/jianbo/paintings/westlake2/Yin+and+Yang+of+it.jpg
	k=$(basename ${filename##*/} .jpg)
	if [ ! -f $path/../westlake2-sig256/$k.supgrad.mat ]
	then
		qsub -v file=$filename job_submit.sh	
	fi
done
