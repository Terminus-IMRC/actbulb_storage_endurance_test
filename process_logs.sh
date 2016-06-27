#!/usr/bin/env bash

usage() {
	echo "Usage: $0 LOGDIR"
}

if test "$#" -ne 1; then
	echo "error: Invalid the number of the arguments"
	usage
	exit 1
fi


LOGDIR="$1"
TMP=$(mktemp)
OFFSET=0
for log in `\ls -1 "$LOGDIR" | sort -g | sed "s\"^\"$LOGDIR/\"g"`; do
	gawk --lint -v "OFFSET=$OFFSET" -f extract_log.gawk <"$log" | tee "$TMP"
	OFFSET=$(tail -n 1 "$TMP" | cut -d ' ' -f 1)
done

rm -f "$TMP"
