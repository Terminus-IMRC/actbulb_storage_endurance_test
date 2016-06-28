#!/usr/bin/env gnuplot

set terminal pdfcairo enhanced color
set yrange [0:]
set grid
set xlabel "Total written data [B]"
set ylabel "Speed [B/s]"


set output "result1.pdf"
plot \
	"result.txt" using 1:2 with lines title "Download", \
	"" using 1:3 with lines title "Write", \
	"" using 1:4 with lines title "Read"

set output "result2.pdf"
plot \
	"result.txt" using 1:3 with lines title "Write", \
	"" using 1:4 with lines title "Read"
