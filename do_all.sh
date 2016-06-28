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

./process_logs.sh "$LOGDIR" >result.txt
./plot.gp
