/*
 * actbulb_storage_endurance_test -- eMMC endurance test for ActBulb
 *
 * Copyright (c) 2016 Sugizaki Yukimasa. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <curl/curl.h>
#include <errno.h>

struct chunk {
	size_t size;
	size_t size_written;
	uint8_t *buf;
};

static void error(const char *file, const int line, const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "%s:%d: ", file, line);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

static void usage(FILE *outfp, const char *progname)
{
	fprintf(outfp, "Usage: %s MAX_SIZE FILES_TO_DOWNLOAD...\n", progname);
}

static void process_args(size_t *max_size, uint8_t **buf, int *nurls, char ***urls, char ***basenames, int argc, char *argv[])
{
	int i;

	if (argc < 3) {
		error(__FILE__, __LINE__, "error: Insufficient the number of the arguments\n");
		usage(stderr, argv[0]);
		exit(EXIT_FAILURE);
	}

	*max_size = atoi(argv[1]);
	if ((*buf = malloc(*max_size)) == NULL) {
		error(__FILE__, __LINE__, "error: Failed to allocate buffer of %zu bytes\n", *max_size);
		exit(EXIT_FAILURE);
	}

	*nurls = argc - 2;
	if ((*urls = malloc(*nurls * sizeof(char*))) == NULL) {
		error(__FILE__, __LINE__, "error: Failed to allocate urls buffer of %d elements\n", *nurls);
		exit(EXIT_FAILURE);
	}
	if ((*basenames = malloc(*nurls * sizeof(char*))) == NULL) {
		error(__FILE__, __LINE__, "error: Failed to allocate basenames buffer of %d elements\n", *nurls);
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < *nurls; i ++) {
		char *p = argv[i + 2], *q = NULL;
		if (((*urls)[i] = strdup(p)) == NULL) {
			error(__FILE__, __LINE__, "strdup: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if ((q = basename(p)) == NULL) {
			error(__FILE__, __LINE__, "basename: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		if (((*basenames)[i] = strdup(q)) == NULL) {
			error(__FILE__, __LINE__, "strdup: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
}

size_t download_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	struct chunk *chunk = (struct chunk*) userdata;
	size_t realsize = size * nmemb;
	size_t writesize;

	if (realsize == 0)
		return 0;

	if (chunk->size_written + realsize > chunk->size) {
		error(__FILE__, __LINE__, "error: size_written(%zu) + realsize(%zu) > size(%zu)\n", chunk->size_written, realsize, chunk->size);
		writesize = chunk->size - chunk->size_written;
	} else
		writesize = realsize;

	memcpy(&(chunk->buf[chunk->size_written]), ptr, writesize);

	chunk->size_written += writesize;

	return writesize;
}

static _Bool download(const char *url_arg, struct chunk *chunkp)
{
	CURL *curl_handle;
	CURLcode ret;
	char *url;
	_Bool toret = 1;

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();

	url = strdup(url_arg);
	ret = curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	if (ret != CURLE_OK) {
		error(__FILE__, __LINE__, "curl_easy_setopt: CURLOPT_URL: %s\n", curl_easy_strerror(ret));
		goto cleanup;
	}

	ret = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, download_callback);
	if (ret != CURLE_OK) {
		error(__FILE__, __LINE__, "curl_easy_setopt: CURLOPT_WRITEFUNCTION: %s\n", curl_easy_strerror(ret));
		goto cleanup;
	}

	ret = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*) chunkp);
	if (ret != CURLE_OK) {
		error(__FILE__, __LINE__, "curl_easy_setopt: CURLOPT_WRITEDATA: %s\n", curl_easy_strerror(ret));
		goto cleanup;
	}

	ret = curl_easy_perform(curl_handle);
	if (ret != CURLE_OK) {
		error(__FILE__, __LINE__, "curl_easy_perform: %s\n", curl_easy_strerror(ret));
		goto cleanup;
	}

	toret = 0;

cleanup:
	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();

	return toret;
}

static _Bool do_write(const int fd, struct chunk chunk)
{
	int reti;
	ssize_t retss;

	retss = write(fd, chunk.buf, chunk.size_written);
	if (retss == -1) {
		error(__FILE__, __LINE__, "error: write: %s\n", strerror(errno));
		return 1;
	} else if ((size_t) retss != chunk.size_written) {
		error(__FILE__, __LINE__, "error: Short write\n");
		return 1;
	}

	reti = fsync(fd);
	if (reti == -1) {
		error(__FILE__, __LINE__, "error: fsync: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}

static _Bool do_verify(const int fd, struct chunk chunk)
{
	size_t i;
	uint8_t buf[1<<18]; /* Must not be greater than the stack size! */
	ssize_t lastsize;
	off_t reto;
	ssize_t retss;

	reto = lseek(fd, 0, SEEK_SET);
	if (reto == -1) {
		error(__FILE__, __LINE__, "error: lseek: %s\n", strerror(errno));
		return 1;
	}

	for (i = 0; i < chunk.size_written / sizeof(buf); i ++) {
		retss = read(fd, buf, sizeof(buf));
		if (retss == -1) {
			error(__FILE__, __LINE__, "error: read: %s\n", strerror(errno));
			return 1;
		} else if (retss != sizeof(buf)) {
			error(__FILE__, __LINE__, "error: Short read\n");
			return 1;
		}
		if (memcmp(buf, &(chunk.buf[sizeof(buf) * i]), sizeof(buf))) {
			error(__FILE__, __LINE__, "Contents mismatch at i=%zu\n", i);
			return 1;
		}
	}

	/* Or sizeof(buf) * i - chunk.size_written */
	lastsize = 	chunk.size_written % sizeof(buf);
	retss = read(fd, buf, lastsize);
	if (retss == -1) {
		error(__FILE__, __LINE__, "error: read: %s\n", strerror(errno));
		return 1;
	} else if (retss != lastsize) {
		error(__FILE__, __LINE__, "error: Short read (retss(%zs) != lastsize(%zs))\n", retss, lastsize);
		return 1;
	}
	if (memcmp(buf, &(chunk.buf[sizeof(buf) * i]), lastsize)) {
		error(__FILE__, __LINE__, "Contents mismatch at i=%zu\n", i);
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int i;
	int nurls = 0;
	char **urls = NULL, **basenames = NULL;
	struct chunk chunk;
	int reti;

	process_args(&chunk.size, &chunk.buf, &nurls, &urls, &basenames, argc, argv);

	printf("chunk.size = %zu\n", chunk.size);
	printf("chunk.buf = %p\n", (void*) chunk.buf);
	printf("nurls = %d\n", nurls);
	for (i = 0; i < nurls; i ++) {
		printf("urls[%d] = %s\n", i, urls[i]);
		printf("basenames[%d] = %s\n", i, basenames[i]);
	}
	fflush(stdout);

	for (i = 0; i < nurls; i ++) {
		int fd;
		struct timeval start, end;

		chunk.size_written = 0;

		gettimeofday(&start, NULL);
		if (download(urls[i], &chunk))
			goto cleanup;
		gettimeofday(&end, NULL);

		printf("chunk.size_written(%d): %zu [B]\n", i, chunk.size_written);
		printf("Download time(%d): %f [s]\n", i, (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6);
		printf("Download speed(%d): %g [B/s]\n", i, chunk.size_written / ((end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6));
		fflush(stdout);

		fd = open(basenames[i], O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
		if (fd == -1) {
			error(__FILE__, __LINE__, "error: open: %s\n", strerror(errno));
			goto cleanup;
		}

		gettimeofday(&start, NULL);
		if (do_write(fd, chunk))
			goto cleanup;
		gettimeofday(&end, NULL);

		printf("Write time(%d): %f [s]\n", i, (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6);
		printf("Write speed(%d): %g [B/s]\n", i, chunk.size_written / ((end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6));
		fflush(stdout);

		if (do_verify(fd, chunk))
			goto cleanup;

		reti = close(fd);
		if (reti == -1) {
			error(__FILE__, __LINE__, "error: close: %s\n", strerror(errno));
			goto cleanup;
		}
	}

cleanup:
	for (i = 0; i < nurls; i ++) {
		free(basenames[i]);
		free(urls[i]);
	}
	free(basenames);
	free(urls);
	free(chunk.buf);

	return 0;
}
