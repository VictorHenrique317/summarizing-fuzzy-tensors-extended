#!/bin/sh

if [ $# != 1 ]
then
    printf "Usage: $0 planted-patterns

Assessed patterns on the standard input.
"
    exit 64
fi

unflatten_field_i ()
{
    awk -v field_pos=$i '
{
    n = split($field_pos, a, ",")
    for (i = 1; i <= n; ++i) {
        $field_pos = a[i]
        print } }
!n {
    print }' $1 > $TMP/tmp
}

TMP=`mktemp -dt quality.sh.XXXXXX`
trap "rm -r $TMP 2>/dev/null" 0

# Identifying the pattern with the (n + 1)-th field
awk '{ print $0, NR }' "$1" > $TMP/correct
n=$(head -1 "$1" | wc -w)
awk -v id_field=$(expr $n + 1) '
{
    $id_field = NR
    NF = id_field
    print }' > $TMP/discovered

if [ ! -s $TMP/correct -a -s $TMP/discovered ]
then
    printf '0
'
    exit
fi

# Listing the tuples in every pattern
for i in $(seq $n)
do
    unflatten_field_i $TMP/correct
    mv $TMP/tmp $TMP/correct
    unflatten_field_i $TMP/discovered
    mv $TMP/tmp $TMP/discovered
done

# Tuples of any pattern in an individual file
mkdir $TMP/correct-indiv $TMP/discovered-indiv
awk '
FNR == 1 {
    out = FILENAME "-indiv/" }

{
    id = $NF
    --NF
    print > out id }' $TMP/correct $TMP/discovered
for pattern in $TMP/correct-indiv/* $TMP/discovered-indiv/*
do
    sort -o $pattern $pattern
done

# True-positive tuples
for p in $TMP/correct-indiv/*
do
    max=0
    for x in $TMP/discovered-indiv/*
    do
	inter=$(comm -12 $p $x | wc -l)
	union=$(sort -mu $p $x | wc -l)
	if [ $(echo "$inter / $union > $max" | bc -l) = 1 ]
	then
	    max=$(echo "$inter / $union" | bc -l)
	    argmax=$x
	fi
    done
    comm -12 $p $argmax >> $TMP/true-positive
done

echo "$(sort -u $TMP/true-positive | wc -l) / $(sort -u $TMP/correct-indiv/* $TMP/discovered-indiv/* | wc -l)" | bc -l
