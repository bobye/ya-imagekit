path=/gpfs/home/jxy198/group/jianbo/paintings/Westlake

for filename in `ls -1 $path/westlake_by_artists/*/*.jpg`
do
	k=`echo $filename | sed 's/westlake_by_artists/signature/g' | sed 's/.jpg/.supgrad.mat/g'`
	if [ ! -f "$k" ]
	then
		qsub -v ifile="$filename",ofile="$k" job_submit.sh	
	fi
#	echo $filename
done
