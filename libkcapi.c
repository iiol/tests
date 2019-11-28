#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <kcapi.h>

int
main(void)
{
	int idx, ret;
	uint8_t *in, *out;
	struct kcapi_handle **hdls;

	hdls = malloc(2*sizeof (struct kcapi_handle*));

	idx = kcapi_cipher_init(hdls, "ecb(aes)", 0);
	if (idx < 0) {
		errno = -idx;
		perror("kcapi_cipher_init()");
		return 1;
	}

	ret = kcapi_cipher_setkey(hdls[idx], (uint8_t*) "1234567890123456", 16);
	if (ret < 0) {
		errno = -ret;
		perror("kcapi_cipher_setkey()");
		return 1;
	}

	in  = malloc(32);
	out = malloc(32);
	memset(in,  0, 32);
	memset(out, 0, 32);
	strcpy((char*) in, "Hello World");

	ret = kcapi_cipher_encrypt(hdls[idx], in, 32, NULL, out, 32, 0);
	if (ret < 0) {
		errno = -ret;
		perror("kcapi_cipher_encrypt()");
		return 1;
	}

	kcapi_cipher_destroy(hdls[idx]);

	return 0;
}
