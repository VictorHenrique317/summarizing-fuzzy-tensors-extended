#!/bin/sh

cd /app/datasets/retweets
mkdir 2d
mkdir 3d
# Weekly influences (3D)
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
END { for (t in tuple) print t, tuple[t] }' teams - retweets > 3d/retweets

# Compute the per-team multiplicative coefficients, normalize so that every team has the same weight and order in descending order of normalized value so that 3d/normalized_retweets can be in argument of nclusterbox's -p option and -m can be used, turn fuzzy using a sigmoid: threshold = 7; growth rate = .5
gawk '{ total[$3] += $4 }
END { for (team in total) avg += total[team]; avg /= length(total); for (team in total) print team, avg / total[team] }' 3d/retweets | gawk 'ARGIND == 1 { coeff[$1] = $2 }
ARGIND == 2 { team[$1] = $2 }
ARGIND == 3 { $4 *= coeff[$3]; if ($3 in team) $3 = team[$3]; print; $4 = 1 / (1 + exp(.5 * (7 - $4))); print > "3d/influences" }' - teams 3d/retweets 2> /dev/null | sort -k 4gr | head -1000 | cut -d ' ' -f -3 > 3d/init_patterns

# Overall influences (2D)
# Compute the per-team multiplicative coefficients, normalize so that every team has the same weight and order in descending order of normalized value so that normalized_retweets can be in argument of nclusterbox's -p option and -m can be used, turn fuzzy using a sigmoid: threshold = 7 * 12; growth rate = .5 / 12
gawk '{ tuple[$3 " " $2] += $4 } END { for (t in tuple) print t, tuple[t] }' 3d/retweets > 2d/retweets

gawk '{ total[$1] += $3 }
END { for (team in total) avg += total[team]; avg /= length(total); for (team in total) print team, avg / total[team] }' 2d/retweets | gawk 'ARGIND == 1 { coeff[$1] = $2 }
ARGIND == 2 { team[$1] = $2 }
ARGIND == 3 { $3 *= coeff[$1]; if ($1 in team) $1 = team[$1]; print; $3 = 1 / (1 + exp(.04166666666666667 * (84 - $3))); print > "2d/influences" }' - teams 2d/retweets 2> /dev/null | sort -k 3gr | head -1000 | cut -d ' ' -f -2 > 2d/init_patterns

cd /app/script
