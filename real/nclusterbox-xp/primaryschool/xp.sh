#!/bin/sh

mkdir -p summaries iterations

# Aggregate per hour and turn symmetric (remark: in contacts, $2 < $3 always stands)
gawk '{ $1 = int($1 / 3600); $2 = $2 ":" $4; $3 = $3 ":" $5; NF -= 2 }
!($0 in edges) { print; print $1, $3, $2; edges[$0]; hours[$1]; vertices[$2]; vertices[$3] } END { for (h in hours) for (v in vertices) print h, v, v }' contacts > per-hour

# Mining
printf "# alg\tparsing\tshifting\tmodifying\tcandidates\treduction\tselected\tselection\ttotal\n" > perfs
printf "NclusterBox-all\t" >> perfs
../nclusterbox/nclusterbox per-hour -bo summaries/nclusterbox --ps --pr >> perfs
# printf "NclusterBox-no-perf-improvement\t" >> perfs
# ../nclusterbox/nclusterbox-no-perf-improvement per-hour -bo /dev/null -m 1000 --ps --pr >> perfs
# printf "TriclusterBox\t" >> perfs
# ../nclusterbox/oldclusterbox per-hour -bo summaries/biclusterbox --ps --pr >> perfs

# Testing stability
for i in $(seq 30)
do
    printf "NclusterBox\t" >> perfs
    ../nclusterbox/nclusterbox per-hour -bo iterations/$i -m 1000
done >> perfs
gawk 'ARGIND == 1 && NR < 14 { NF = 3; patterns[$0] } ARGIND != 1 { NF = 3; if ($0 in patterns) ++patterns[$0] } END { for (p in patterns) print p, patterns[p] / 30 }' summaries/nclusterbox iterations/* > stability
