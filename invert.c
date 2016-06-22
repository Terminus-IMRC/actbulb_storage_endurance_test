#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

int main()
{
	uint8_t buf[1 << 20];
	ssize_t i;
	ssize_t retss_read, retss_write;

	while ((retss_read = read(STDIN_FILENO, buf, sizeof(buf))) != -1) {
		if (retss_read == 0)
			break;

		for (i = 0; i < retss_read; i ++)
			buf[i] = ~buf[i];

		retss_write = write(STDOUT_FILENO, buf, retss_read);
		if (retss_write == -1) {
			fprintf(stderr, "%s:%d: error: write: %s\n", __FILE__, __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		} else if (retss_write != retss_read) {
			fprintf(stderr, "%s:%d: error: Short or extra write at i=%zd (%zd != %zd)\n", __FILE__, __LINE__, i, retss_write, retss_read);
			exit(EXIT_FAILURE);
		}
	}
	if (retss_read == -1) {
		fprintf(stderr, "%s:%d: error: write: %s\n", __FILE__, __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return 0;
}
