#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <openssl/whrlpool.h>

static void crypto_bench(const char *name, void (*testfunc)(const unsigned char *d, const size_t n), const unsigned char *d, const size_t n)
{
	struct timeval start, end;

	gettimeofday(&start, NULL);
	(*testfunc)(d, n);
	gettimeofday(&end, NULL);

	printf("%s: %f [s]\n", name, (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6);
}

static void md4_test(const unsigned char *d, const size_t n)
{
	MD4(d, n, NULL);
}

static void md5_test(const unsigned char *d, const size_t n)
{
	MD5(d, n, NULL);
}

static void ripemd160_test(const unsigned char *d, const size_t n)
{
	RIPEMD160(d, n, NULL);
}

static void sha1_test(const unsigned char *d, const size_t n)
{
	SHA1(d, n, NULL);
}

static void sha224_test(const unsigned char *d, const size_t n)
{
	SHA224(d, n, NULL);
}

static void sha256_test(const unsigned char *d, const size_t n)
{
	SHA256(d, n, NULL);
}

static void sha384_test(const unsigned char *d, const size_t n)
{
	SHA384(d, n, NULL);
}

static void sha512_test(const unsigned char *d, const size_t n)
{
	SHA512(d, n, NULL);
}

static void whirlpool_test(const unsigned char *d, const size_t n)
{
	WHIRLPOOL(d, n, NULL);
}

int main()
{
	const size_t n = 260e6;

	unsigned char *d;

	d = malloc(n);
	if (d == NULL) {
		fprintf(stderr, "error: Failed to allocate memory of %zu bytes\n", n);
		exit(EXIT_FAILURE);
	}

	crypto_bench("MD4      ", md4_test, d, n);
	crypto_bench("MD5      ", md5_test, d, n);
	crypto_bench("RIPEMD160", ripemd160_test, d, n);
	crypto_bench("SHA1     ", sha1_test, d, n);
	crypto_bench("SHA224   ", sha224_test, d, n);
	crypto_bench("SHA256   ", sha256_test, d, n);
	crypto_bench("SHA384   ", sha384_test, d, n);
	crypto_bench("SHA512   ", sha512_test, d, n);
	crypto_bench("WHIRLPOOL", whirlpool_test, d, n);

	free(d);

	return 0;
}
