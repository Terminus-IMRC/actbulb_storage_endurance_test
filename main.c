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

static void usage(FILE *outfp, const char *progname)
{
	fprintf(outfp, "Usage: %s MAX_SIZE FILES_TO_DOWNLOAD...\n", progname);
}

static void process_args(size_t *max_size, uint8_t **buf, int *nurls, char ***urls, char ***basenames, int argc, char *argv[])
{
	int i;

	if (argc < 3) {
		fprintf(stderr, "%s: error: Insufficient the number of the arguments\n", argv[0]);
		usage(stderr, argv[0]);
		exit(EXIT_FAILURE);
	}

	*max_size = atoi(argv[1]);
	if ((*buf = malloc(*max_size)) == NULL) {
		fprintf(stderr, "%s: error: Failed to allocate buffer of %lu bytes", argv[0], *max_size);
		exit(EXIT_FAILURE);
	}

	*nurls = argc - 2;
	if ((*urls = malloc(*nurls * sizeof(char*))) == NULL) {
		fprintf(stderr, "%s: error: Failed to allocate urls buffer of %d elements", argv[0], *nurls);
		exit(EXIT_FAILURE);
	}
	if ((*basenames = malloc(*nurls * sizeof(char*))) == NULL) {
		fprintf(stderr, "%s: error: Failed to allocate basenames buffer of %d elements", argv[0], *nurls);
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < *nurls; i ++) {
		char *p = argv[i + 2], *q = NULL;
		if (((*urls)[i] = strdup(p)) == NULL) {
			fprintf(stderr, "%s: strdup: %s\n", argv[0], strerror(errno));
			exit(EXIT_FAILURE);
		}
		if ((q = basename(p)) == NULL) {
			fprintf(stderr, "%s: basename: %s\n", argv[0], strerror(errno));
			exit(EXIT_FAILURE);
		}
		if (((*basenames)[i] = strdup(q)) == NULL) {
			fprintf(stderr, "%s: strdup: %s\n", argv[0], strerror(errno));
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
		fprintf(stderr, "error: size_written(%lu) + realsize(%lu) > size(%lu)\n", chunk->size_written, realsize, chunk->size);
		writesize = chunk->size - chunk->size_written;
	} else
		writesize = realsize;

	memcpy(&(chunk->buf[chunk->size_written]), ptr, writesize);

	chunk->size_written += writesize;

	return writesize;
}

static _Bool download(const char *url_arg, struct chunk *chunkp, const char *progname)
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
		fprintf(stderr, "%s: curl_easy_setopt: CURLOPT_URL: %s\n", progname, curl_easy_strerror(ret));
		goto cleanup;
	}

	ret = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, download_callback);
	if (ret != CURLE_OK) {
		fprintf(stderr, "%s: curl_easy_setopt: CURLOPT_WRITEFUNCTION: %s\n", progname, curl_easy_strerror(ret));
		goto cleanup;
	}

	ret = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*) chunkp);
	if (ret != CURLE_OK) {
		fprintf(stderr, "%s: curl_easy_setopt: CURLOPT_WRITEDATA: %s\n", progname, curl_easy_strerror(ret));
		goto cleanup;
	}

	ret = curl_easy_perform(curl_handle);
	if (ret != CURLE_OK) {
		fprintf(stderr, "%s: curl_easy_perform: %s\n", progname, curl_easy_strerror(ret));
		goto cleanup;
	}

	toret = 0;

cleanup:
	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();

	return toret;
}

int main(int argc, char *argv[])
{
	int i;
	int nurls = 0;
	char **urls = NULL, **basenames = NULL;
	struct chunk chunk;

	process_args(&chunk.size, &chunk.buf, &nurls, &urls, &basenames, argc, argv);

	printf("chunk.size = %lu\n", chunk.size);
	printf("chunk.buf = %p\n", (void*) chunk.buf);
	printf("nurls = %d\n", nurls);
	for (i = 0; i < nurls; i ++) {
		printf("urls[%d] = %s\n", i, urls[i]);
		printf("basenames[%d] = %s\n", i, basenames[i]);
	}

	for (i = 0; i < nurls; i ++) {
		int fd;
		struct timeval start, end;
		int reti;
		_Bool retb;
		ssize_t retss;

		chunk.size_written = 0;

		gettimeofday(&start, NULL);
		retb = download(urls[i], &chunk, argv[0]);
		if (retb) {
			fprintf(stderr, "%s: error: download\n", argv[0]);
			goto cleanup;
		}
		gettimeofday(&end, NULL);

		printf("chunk.size_written(%d): %lu [B]\n", i, chunk.size_written);
		printf("Download time(%d): %f [s]\n", i, (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6);
		printf("Download speed(%d): %g [B/s]\n", i, chunk.size_written / ((end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6));

		fd = open("/dev/null", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
		if (fd == -1) {
			fprintf(stderr, "error: open: %s\n", strerror(errno));
			goto cleanup;
		}

		gettimeofday(&start, NULL);
		retss = write(fd, chunk.buf, chunk.size_written);
		if (retss == -1) {
			fprintf(stderr, "error: write: %s\n", strerror(errno));
			goto cleanup;
		} else if ((size_t) retss != chunk.size_written) {
			fprintf(stderr, "error: Short write\n");
			goto cleanup;
		}
		reti = fsync(fd);
		if (reti == -1) {
			fprintf(stderr, "error: fsync: %s\n", strerror(errno));
			goto cleanup;
		}
		gettimeofday(&end, NULL);

		reti = close(fd);
		if (reti == -1) {
			fprintf(stderr, "error: close: %s\n", strerror(errno));
			goto cleanup;
		}

		printf("Write time(%d): %f [s]\n", i, (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6);
		printf("Write speed(%d): %g [B/s]\n", i, chunk.size_written / ((end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6));
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
