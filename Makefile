CFLAGS += -Wall -Wextra -O2

RM := rm -f

all: main invert crypto_bench

main: LDFLAGS += -lcurl -lcrypto
main: main.o

invert: invert.o

crypto_bench: LDFLAGS += -lcrypto
crypto_bench: crypto_bench.o

.PHONY: clean
clean:
	$(RM) crypto_bench
	$(RM) invert
	$(RM) main
	$(RM) crypto_bench.o
	$(RM) invert.o
	$(RM) main.o
	$(RM) urls2.txt
	$(RM) urls1.txt
