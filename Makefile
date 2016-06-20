CFLAGS := -Wall -Wextra -O2 -ansi -pedantic
LDFLAGS := -lcurl

RM := rm -f

main: main.o

.PHONY:
clean:
	$(RM) main
	$(RM) main.o
	$(RM) urls2.txt
	$(RM) urls1.txt
