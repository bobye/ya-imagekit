#!/bin/sh

dir=../data
output=../cf.txt

rm -f $output

IFS=$'\n'
for i in `ls -1 $dir`;
do
  echo $dir/$i $output
 ./extractColorFactors $dir/$i >> $output
done
