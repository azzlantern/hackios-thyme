#ifndef _UUID_UUID_H
#define _UUID_UUID_H

typedef unsigned char uuid_t[16];
typedef char uuid_string_t[37];

static inline void uuid_generate(uuid_t out) {
	unsigned int r;
	for (int i = 0; i < 16; i += 4) {
		r = (unsigned int)__builtin_ia32_rdtsc();
		out[i] = r; out[i+1] = r >> 8; out[i+2] = r >> 16; out[i+3] = r >> 24;
	}
	out[6] = (out[6] & 0x0f) | 0x40;
	out[8] = (out[8] & 0x3f) | 0x80;
}

#endif
