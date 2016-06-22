CFLAGS += -Wall -Wextra -O2

RM := rm -f

all: main invert

main: LDFLAGS += -lcurl
main: main.o

invert: invert.o

.PHONY: clean
clean:
	$(RM) invert
	$(RM) main
	$(RM) invert.o
	$(RM) main.o
	$(RM) urls2.txt
	$(RM) urls1.txt
