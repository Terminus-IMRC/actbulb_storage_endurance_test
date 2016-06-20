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

MAXSIZE=766213793

if test -z "$INV"; then
	URLS1="urls1.txt"
	URLS2="urls2.txt"
else
	URLS1="urls2.txt"
	URLS2="urls1.txt"
fi

cat <<END >"urls1.txt"
http://192.168.11.200/~imrc/sd_movie/out1
http://192.168.11.200/~imrc/sd_movie/out2
http://192.168.11.200/~imrc/sd_movie/out3
http://192.168.11.200/~imrc/sd_movie/out4
http://192.168.11.200/~imrc/sd_movie/out5
http://192.168.11.200/~imrc/sd_movie/out6
END

cat <<END >"urls2.txt"
http://192.168.11.200/~imrc/sd_movie/out1i
http://192.168.11.200/~imrc/sd_movie/out2i
http://192.168.11.200/~imrc/sd_movie/out3i
http://192.168.11.200/~imrc/sd_movie/out4i
http://192.168.11.200/~imrc/sd_movie/out5i
END


LOCKFILE="$(mktemp)"

echo "Running tests for $NLOOPS times"
echo "Remove $LOCKFILE to stop them"
for i in `seq "$NLOOPS"`; do
	if test "$((i % 2))" -eq 1; then
		URLS="$URLS1"
	else
		URLS="$URLS2"
	fi
	./main "$MAXSIZE" `cat "$URLS"`
	if test "$?" -ne 0; then
		echo "error: main returned non-zero" >&2
		break
	fi
	if ! test -e "$LOCKFILE"; then
		echo "Exitting the test loop since the lockfile is removed"
		break
	fi
done

rm -f "$LOCKFILE"
