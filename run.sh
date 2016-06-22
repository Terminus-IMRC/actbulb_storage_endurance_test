#!/usr/bin/env bash

if test "$#" -ne 1 -a "$#" -ne 2; then
	echo "error: Invalid the number of the arguments" >&2
	echo "Usage: $0 NLOOPS [SET_IF_DO_INV]"
	exit 1
fi

echo -n "Press return to continue> "
read


NLOOPS="$1"
INV="$2"

MAXSIZE=260772658

if test -z "$INV"; then
	URLS1="urls1.txt"
	URLS2="urls2.txt"
else
	URLS1="urls2.txt"
	URLS2="urls1.txt"
fi

cat <<END >"urls1.txt"
http://192.168.11.200/~imrc/sd_movie/out1-1
http://192.168.11.200/~imrc/sd_movie/out1-2
http://192.168.11.200/~imrc/sd_movie/out2-1
http://192.168.11.200/~imrc/sd_movie/out2-2
http://192.168.11.200/~imrc/sd_movie/out3-1
http://192.168.11.200/~imrc/sd_movie/out3-2
http://192.168.11.200/~imrc/sd_movie/out4-1
http://192.168.11.200/~imrc/sd_movie/out4-2
http://192.168.11.200/~imrc/sd_movie/out5-1
http://192.168.11.200/~imrc/sd_movie/out5-2
http://192.168.11.200/~imrc/sd_movie/out6-1
http://192.168.11.200/~imrc/sd_movie/out6-2
END

cat <<END >"urls2.txt"
http://192.168.11.200/~imrc/sd_movie/out1-1i
http://192.168.11.200/~imrc/sd_movie/out1-2i
http://192.168.11.200/~imrc/sd_movie/out2-1i
http://192.168.11.200/~imrc/sd_movie/out2-2i
http://192.168.11.200/~imrc/sd_movie/out3-1i
http://192.168.11.200/~imrc/sd_movie/out3-2i
http://192.168.11.200/~imrc/sd_movie/out4-1i
http://192.168.11.200/~imrc/sd_movie/out4-2i
http://192.168.11.200/~imrc/sd_movie/out5-1i
http://192.168.11.200/~imrc/sd_movie/out5-2i
http://192.168.11.200/~imrc/sd_movie/out6-1i
http://192.168.11.200/~imrc/sd_movie/out6-2i
END

clean_files() {
	rm -f out*-*
}


LOCKFILE="$(mktemp)"
LOG="log/$(date '+%Y:%m:%d-%H:%M:%S')"

mkdir -p log

(
echo "Running tests for $NLOOPS times"
echo "Remove $LOCKFILE to stop them"
for i in `seq "$NLOOPS"`; do
	date
	echo "i = $i"
	clean_files
	sync; sync; sync
	if test "$((i % 2))" -eq 1; then
		URLS="$URLS1"
	else
		URLS="$URLS2"
	fi
	./main "$MAXSIZE" `cat "$URLS" | shuf`
	if test "$?" -ne 0; then
		echo "error: main returned non-zero" >&2
		break
	fi
	if ! test -e "$LOCKFILE"; then
		echo "Exitting the test loop since the lockfile is removed"
		break
	fi
done
date
) 2>&1 | tee "$LOG"

rm -f "$LOCKFILE"
