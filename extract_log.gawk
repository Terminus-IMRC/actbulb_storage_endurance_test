#!/usr/bin/env gawk --lint -v OFFSET=0 -f
# Warning: The shebang above is only for testing!

BEGIN {
	N_cs = split("chunk.size_written", A_cs, " ");
	N_ds = split("Download speed", A_ds, " ");
	N_ws = split("Write speed", A_ws, " ");
	N_rs = split("Read speed", A_rs, " ");

	C_cs = 0;
	C_ds = 0;
	C_ws = 0;
	C_rs = 0;

	R_cs[1] = 0;
	R_ds[1] = 0;
	R_ws[1] = 0;
	R_rs[1] = 0;
}

function strlen1cmp(s1, s2) {
	len = length(s1);
	if (len > length(s2))
		return 0;
	return (substr(s1, 1, len) == substr(s2, 1, len));
}

function is_match_string(A, N, C, R, offset) {
	is_diff = 0;
	for (i = 1; i <= N; i ++) {
		if (!strlen1cmp(A[i], $i)) {
			is_diff = 1;
			break;
		}
	}
	if (is_diff == 0) {
		C ++;
		R[C] = $offset;
	}
	return C;
}

{
	C_cs = is_match_string(A_cs, N_cs, C_cs, R_cs, 2);
	C_ds = is_match_string(A_ds, N_ds, C_ds, R_ds, 3);
	C_ws = is_match_string(A_ws, N_ws, C_ws, R_ws, 3);
	C_rs = is_match_string(A_rs, N_rs, C_rs, R_rs, 3);
}

END {
	if (C_rs == 0) {
		if ((C_cs != C_ds) || (C_ds != C_ws)) {
			print "error: Counts differ";
			exit(1);
		}
		is_read_exists = 0;
	} else {
		if ((C_cs != C_ds) || (C_ds != C_ws) || (C_ws != C_rs)) {
			print "error: Counts differ";
			exit(1);
		}
		is_read_exists = 1;
	}

	for (i = 1; i < C_ds; i ++) {
		OFFSET += R_cs[i];
		printf("%d", OFFSET);
		printf(" %g", R_ds[i]);
		printf(" %g", R_ws[i]);
		if (is_read_exists)
			printf(" %g", R_rs[i]);
		printf("\n");
	}
}
