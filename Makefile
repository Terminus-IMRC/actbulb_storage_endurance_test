CFLAGS := -Wall -Wextra -O2
LDFLAGS := -lcurl

RM := rm -f

all: main invert

main: main.o

invert: invert.o

.PHONY:
clean:
	$(RM) invert
	$(RM) main
	$(RM) invert.o
	$(RM) main.o
	$(RM) urls2.txt
	$(RM) urls1.txt
