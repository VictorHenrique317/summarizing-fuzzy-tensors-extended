#!/bin/sh

printf 'Building Bi/TriclusterBox\n'
cd nclusterbox
sed -i -e '/#define UPDATE_SUMS/ s:^:/* :' -e '/#define UPDATE_SUMS/ s:$: */:' -e '/#define NB_OF_PATTERNS/ s:^/\* ::' -e '/#define NB_OF_PATTERNS/ s: \*/$::' -e '/#define TIME/ s:^/\* ::' -e '/#define TIME/ s: \*/$::' -e '/#define DETAILED_TIME/ s:^/\* ::' -e '/#define DETAILED_TIME/ s: \*/$::' -e '/#define GNUPLOT/ s:^/\* ::' -e '/#define GNUPLOT/ s: \*/$::' Parameters.h
make -j > /dev/null
mv nclusterbox nclusterbox-no-update-sum
printf \"$PWD\"'/nclusterbox-no-update-sum -f "$@"\n' > nclusterbox-no-perf-improvement
printf \"$PWD\"'/slice-input "$1" 1 | '\"$PWD\"'/nclusterbox-no-update-sum -fj 1 -p - "$@"\n' > oldclusterbox
chmod +x nclusterbox-no-perf-improvement oldclusterbox

printf 'Building NclusterBox\n'
sed -i -e '/#define UPDATE_SUMS/ s:^/\* ::' -e '/#define UPDATE_SUMS/ s: \*/$::' Parameters.h
make -j > /dev/null
sed -i -e '/#define NB_OF_PATTERNS/ s:^:/* :' -e '/#define NB_OF_PATTERNS/ s:$: */:' -e '/#define TIME/ s:^:/* :' -e '/#define TIME/ s:$: */:' -e '/#define DETAILED_TIME/ s:^:/* :' -e '/#define DETAILED_TIME/ s:$: */:' -e '/#define GNUPLOT/ s:^:/* :' -e '/#define GNUPLOT/ s:$: */:' Parameters.h

for data in synthetic retweets primaryschool
do
    printf "Summarizing $data\n"
    cd ../$data
    ./xp.sh
done
