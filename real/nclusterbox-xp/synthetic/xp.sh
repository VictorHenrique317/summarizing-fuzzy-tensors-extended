#!/bin/sh

printf "# n\tnb_of_correct\talg\tparsing\tshifting\tmodifying\tcandidates\treduction\tselected\tselection\ttotal\n" > perfs
for i in $(seq 30)
do
    mkdir -p iterations/2D/$i iterations/3D/$i
    yes '50 50' | head | gennsets '1000 1000' /dev/stdin > iterations/2D/$i/patterns
    yes '5 5 5' | head | gennsets '100 100 100' /dev/stdin > iterations/3D/$i/patterns
    for j in 1 2 4 8 16
    do
	num-noise '1000 1000' $j 0 < iterations/2D/$i/patterns > iterations/2D/$i/data
	printf "2\t$j\tNclusterBox\t"
	../nclusterbox/nclusterbox iterations/2D/$i/data -o iterations/2D/$i/summary -m 1000 --mss 10
	num-noise '100 100 100' $j 0 < iterations/3D/$i/patterns > iterations/3D/$i/data
	printf "3\t$j\tNclusterBox\t"
	../nclusterbox/nclusterbox iterations/3D/$i/data -o iterations/3D/$i/summary -m 1000 --mss 10
    done >> perfs
done
