#!/bin/sh

mkdir -p weekly/summaries overall/summaries

# Weekly influences
# Associate a week number to each day in the studied period, select the records in the considered period and aggregate the days into weeks
pastTheEnd="2014-04-07"		# Monday after the last Sunday to consider
it="2014-01-13"			# First Monday to consider
week=3				# The first week is the third week of 2014
while [ "$it" != "$pastTheEnd" ]
do
    for day in $(seq 7)
    do
	echo $it $week
	it=$(date -ud "$it + 1 day" +%Y-%m-%d)
    done
    week=$(expr $week + 1)
done | gawk 'BEGIN { SUBSEP = " " }
ARGIND == 1 { teams[$1] = $2 }
ARGIND == 2 { date2week[$1] = $2 }
ARGIND == 3 { if ($1 in date2week) { if ($3 in teams) $3 = teams[$3]; tuple[date2week[$1], $2, $3] += $4 } }
END { for (t in tuple) print t, tuple[t] }' teams - retweets > weekly/retweets

# Compute the per-team multiplicative coefficients, normalize so that every team has the same weight and order in descending order of normalized value so that weekly/normalized_retweets can be in argument of nclusterbox's -p option and -m can be used, turn fuzzy using a sigmoid: threshold = 7; growth rate = .5
gawk '{ total[$3] += $4 }
END { for (team in total) avg += total[team]; avg /= length(total); for (team in total) print team, avg / total[team] }' weekly/retweets | gawk 'ARGIND == 1 { coeff[$1] = $2 }
ARGIND == 2 { team[$1] = $2 }
ARGIND == 3 { $4 *= coeff[$3]; if ($3 in team) $3 = team[$3]; print; $4 = 1 / (1 + exp(.5 * (7 - $4))); print > "weekly/influences" }' - teams weekly/retweets 2> /dev/null | LC_ALL=C sort -k 4gr | head -1000 | cut -d ' ' -f -3 > weekly/init_patterns

# Overall influences
# Compute the per-team multiplicative coefficients, normalize so that every team has the same weight and order in descending order of normalized value so that normalized_retweets can be in argument of nclusterbox's -p option and -m can be used, turn fuzzy using a sigmoid: threshold = 7 * 12; growth rate = .5 / 12
gawk '{ tuple[$3 " " $2] += $4 } END { for (t in tuple) print t, tuple[t] }' weekly/retweets > overall/retweets

gawk '{ total[$1] += $3 }
END { for (team in total) avg += total[team]; avg /= length(total); for (team in total) print team, avg / total[team] }' overall/retweets | gawk 'ARGIND == 1 { coeff[$1] = $2 }
ARGIND == 2 { team[$1] = $2 }
ARGIND == 3 { $3 *= coeff[$1]; if ($1 in team) $1 = team[$1]; print; $3 = 1 / (1 + exp(.04166666666666667 * (84 - $3))); print > "overall/influences" }' - teams overall/retweets 2> /dev/null | LC_ALL=C sort -k 3gr | head -1000 | cut -d ' ' -f -2 > overall/init_patterns

# Mining
printf "# granularity\talg\tparsing\tshifting\tmodifying\tcandidates\treduction\tselected\tselection\ttotal\n" > perfs
for granularity in weekly overall
do
    printf "$granularity\tNclusterBox\t"
    ../nclusterbox/nclusterbox $granularity/influences -p $granularity/init_patterns -o $granularity/summaries/nclusterbox --ps --pr
    printf "$granularity\tNclusterBox-no-perf-improvement\t"
    ../nclusterbox/nclusterbox-no-perf-improvement $granularity/influences -p $granularity/init_patterns -o /dev/null --ps --pr
    printf "$granularity\tTriclusterBox\t"
    ../nclusterbox/oldclusterbox $granularity/influences -o $granularity/summaries/biclusterbox --ps --pr
done >> perfs
