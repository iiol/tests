#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

// KEY[], STATE1[] and STATE2[] is test values for debug
static const uint8_t KEY[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};

static const uint8_t STATE1[] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
};

static const uint8_t STATE2[] = {
	0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30, 0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a,
};

static const uint8_t SBox[] = {
	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
	0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
	0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
	0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
	0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
	0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
	0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
	0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
	0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
	0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
	0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
	0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
	0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
	0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
	0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16,
};

static const uint8_t InvSBox[] = {
	0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
	0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
	0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
	0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
	0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
	0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
	0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
	0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
	0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
	0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
	0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
	0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
	0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
	0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
	0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
	0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d,
};

uint8_t rcon[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

#define SWAP(a, b) do {a = a ^ b; b = a ^ b; a = a ^ b;} while(0)

#define xtime(x) ((x<<1) ^ (((x>>7) & 1) * 0x1b))

#define mult(x, y) (				\
((y    & 1) * x)			^	\
((y>>1 & 1) * xtime(x))			^	\
((y>>2 & 1) * xtime(xtime(x)))		^	\
((y>>3 & 1) * xtime(xtime(xtime(x))))	^	\
((y>>4 & 1) * xtime(xtime(xtime(xtime(x)))))	\
)

void
AddRoundKey(uint8_t *state, uint8_t *rkey)
{
	uint8_t i;

	for (i = 0; i < 16; ++i)
		state[i] = state[i] ^ rkey[i];
}

void
SubBytes(uint8_t *state)
{
	uint8_t i;

	for (i = 0; i < 16; ++i)
		state[i] = SBox[state[i]];
}

void
InvSubBytes(uint8_t *state)
{
	uint8_t i;

	for (i = 0; i < 16; ++i)
		state[i] = InvSBox[state[i]];
}

void
ShiftRows(uint8_t *state)
{
	uint8_t tmp;

	tmp = state[1];
	state[1] = state[5];
	state[5] = state[9];
	state[9] = state[13];
	state[13] = tmp;

	tmp = state[3];
	state[3] = state[15];
	state[15] = state[11];
	state[11] = state[7];
	state[7] = tmp;

	SWAP(state[2], state[10]);
	SWAP(state[6], state[14]);
}

void
InvShiftRows(uint8_t *state)
{
	uint8_t tmp;

	tmp = state[1];
	state[1] = state[13];
	state[13] = state[9];
	state[9] = state[5];
	state[5] = tmp;

	tmp = state[3];
	state[3] = state[7];
	state[7] = state[11];
	state[11] = state[15];
	state[15] = tmp;

	SWAP(state[2], state[10]);
	SWAP(state[6], state[14]);
}

void
MixColumns(uint8_t *state)
{
	uint8_t i, t[16];

	memcpy(t, state, 16);

	for (i = 0; i < 4; ++i) {
		state[4*i+0] = mult(t[4*i+0], 2) ^ mult(t[4*i+1], 3) ^ t[4*i+2] ^ t[4*i+3];
		state[4*i+1] = mult(t[4*i+1], 2) ^ mult(t[4*i+2], 3) ^ t[4*i+0] ^ t[4*i+3];
		state[4*i+2] = mult(t[4*i+2], 2) ^ mult(t[4*i+3], 3) ^ t[4*i+0] ^ t[4*i+1];
		state[4*i+3] = mult(t[4*i+3], 2) ^ mult(t[4*i+0], 3) ^ t[4*i+1] ^ t[4*i+2];
	}
}

void
InvMixColumns(uint8_t *state)
{
	uint8_t i, t[16];

	memcpy(t, state, 16);

	for (i = 0; i < 4; ++i) {
		state[4*i+0] = mult(t[4*i+0], 0xe) ^ mult(t[4*i+1], 0xb) ^
		    mult(t[4*i+2], 0xd) ^ mult(t[4*i+3], 0x9);
		state[4*i+1] = mult(t[4*i+0], 0x9) ^ mult(t[4*i+1], 0xe) ^
		    mult(t[4*i+2], 0xb) ^ mult(t[4*i+3], 0xd);
		state[4*i+2] = mult(t[4*i+0], 0xd) ^ mult(t[4*i+1], 0x9) ^
		    mult(t[4*i+2], 0xe) ^ mult(t[4*i+3], 0xb);
		state[4*i+3] = mult(t[4*i+0], 0xb) ^ mult(t[4*i+1], 0xd) ^
		    mult(t[4*i+2], 0x9) ^ mult(t[4*i+3], 0xe);
	}
}

void
debug_print(uint8_t *state)
{
	int i;


	for (i = 0; i < 16; ++i) {
		if (i && i % 4 == 0)
			putchar(' ');
		printf("%02x", state[i]);
	}
	putchar('\n');
}

void
encrypt(uint8_t *rkey)
{
	uint8_t i = 0, nc;
	uint8_t *state;

	state = malloc(16);

	while ((nc = read(0, state, 16)) != 0) {
		if (nc < 16)
			memset(state + nc, 0, 16 - nc);

		// Init
		AddRoundKey(state, rkey);

		// First 9 rounds
		for (i = 1; i < 10; ++i) {
			SubBytes(state);
			ShiftRows(state);
			MixColumns(state);
			AddRoundKey(state, &(rkey[16*i]));
		}

		// Last round
		SubBytes(state);
		ShiftRows(state);
		AddRoundKey(state, &(rkey[16*i]));

		write(1, state, 16);
	}
}

void
decrypt(uint8_t *rkey)
{
	uint8_t i, nc;
	uint8_t *state;

	state = malloc(16);

	while ((nc = read(0, state, 16)) != 0) {
		// Init
		AddRoundKey(state, &(rkey[16*10]));

		// First 9 rounds
		for (i = 9; i > 0; --i) {
			InvShiftRows(state);
			InvSubBytes(state);
			AddRoundKey(state, &(rkey[16*i]));
			InvMixColumns(state);
		}

		// Last round
		InvShiftRows(state);
		InvSubBytes(state);
		AddRoundKey(state, &(rkey[16*i]));

		write(1, state, 16);
	}
}

static void
usage_print(char *s)
{
	fprintf(stderr, "Usage: %s <-h | -e | -d>\n", s);
}

int
main(int argc, char **argv)
{
	uint8_t i, j;
	uint8_t *key, *rkey;

	key = malloc(16);
	rkey = malloc(11 * 4 * 4);

	memcpy(key, KEY, 16);		// to remove

	for (i = 0; i < 16; ++i)
		rkey[i] = key[i];

	// Key expansion
	for (i = 4; i < 11 * 4; ++i) {
		for (j = 0; j < 4; ++j) {
			if (i % 4 == 0) {
				// Shift word and SubBytes
				rkey[4*i + j] = SBox[rkey[4*(i-1) + ((j < 3) ? 1+j:0)]];
				// rcon for each 4th word
				rkey[4*i + j] ^= (j == 0) ? rcon[i/4 - 1] : 0;
				rkey[4*i + j] ^= rkey[4*(i - 4) + j];
			}
			else
				rkey[4*i + j] = rkey[4*(i-1) + j] ^ rkey[4*(i-4) + j];
		}
	}

	if (argc != 2) {
		usage_print(argv[0]);
		return 1;
	}

	if (!strcmp(argv[1], "-h")) {
		usage_print(argv[0]);
		return 0;
	}
	else if (!strcmp(argv[1], "-e"))
		encrypt(rkey);
	else if (!strcmp(argv[1], "-d"))
		decrypt(rkey);
}
