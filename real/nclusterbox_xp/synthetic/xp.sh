#!/bin/sh

cd gennsets
make -j > /dev/null
cd ../num-noise
make -j > /dev/null
cd ..

printf "# n\tnb_of_correct\talg\tparsing\tshifting\tmodifying\tcandidates\treduction\tselected\tselection\ttotal\tquality\n" > perfs
for i in $(seq 30)
do
    mkdir -p iterations/2D/$i iterations/3D/$i
    yes 25 25 | head | ./gennsets/gennsets '1000 1000' > iterations/2D/$i/patterns
    yes 5 5 5 | head | ./gennsets/gennsets '100 100 100' > iterations/3D/$i/patterns
    for j in 1 2 4 8 16
    do
	./num-noise/num-noise '1000 1000' $j < iterations/2D/$i/patterns > iterations/2D/$i/data-$j
	./num-noise/num-noise '100 100 100' $j < iterations/3D/$i/patterns > iterations/3D/$i/data-$j
	for n in 2 3
	do
	    perf=$(../nclusterbox/nclusterbox iterations/${n}D/$i/data-$j -o iterations/${n}D/$i/summary-$j-nclusterbox -m 1000)
	    printf "$n\t$j\tNclusterBox\t$perf\t"
	    head iterations/${n}D/$i/summary-$j-nclusterbox | ./quality.sh iterations/${n}D/$i/patterns
	    # perf=$(../nclusterbox/nclusterbox-no-perf-improvement iterations/${n}D/$i/data-$j -o iterations/${n}D/$i/summary-$j-nclusterbox -m 1000)
	    # printf "$n\t$j\tNclusterBox-no-perf-improvement\t$perf\t"
	    # head iterations/${n}D/$i/summary-$j-nclusterbox | ./quality.sh iterations/${n}D/$i/patterns
	    perf=$(../nclusterbox/oldclusterbox iterations/${n}D/$i/data-$j -o iterations/${n}D/$i/summary-$j-oldclusterbox)
	    printf "$n\t$j\tOldclusterBox\t$perf\t"
	    head iterations/${n}D/$i/summary-$j-oldclusterbox | ./quality.sh iterations/${n}D/$i/patterns
	done
    done >> perfs
done
