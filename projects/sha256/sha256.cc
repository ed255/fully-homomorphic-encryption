/*
    MIT License

    Copyright (c) 2020 LekKit https://github.com/LekKit

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

/* Details of the implementation, etc can be found here: https://en.wikipedia.org/wiki/SHA-2
   See sha256.h for short documentation on library usage */

// #include <stdio.h>

#define SIZE 2

#include "sha256.h"

/* Initialization, must be called before any further use */
struct sha256_buff sha256_init();

/* Process block of data of arbitary length, can be used on data streams (files, etc) */
struct sha256_buff sha256_update(struct sha256_buff buff, unsigned char data[SIZE], int size);

/* Produces final hash values (digest) to be read
   If the buffer is reused later, init must be called again */
struct sha256_buff sha256_finalize(struct sha256_buff buff);

/* Read digest into 32-byte binary array */
struct hash sha256_read(struct sha256_buff buff);

// void memcpy(char *dest, const char *src, int n) { 
//     // char *csrc = (char *)src; 
//     // char *cdest = (char *)dest; 
//     // Copy contents of src[] to dest[] 
//     #pragma hls_unroll yes
//     for (int i=0; i<n; i++) {
//         dest[i] = src[i]; 
//     } 
// }
// 
// void memset(char *dest, char ch, int n) { 
//     #pragma hls_unroll yes
//     for (int i=0; i<n; i++) {
//         dest[i] = ch; 
//     } 
// }

struct sha256_buff sha256_init() {
    struct sha256_buff buff; 
    buff.h[0] = 0x6a09e667;
    buff.h[1] = 0xbb67ae85;
    buff.h[2] = 0x3c6ef372;
    buff.h[3] = 0xa54ff53a;
    buff.h[4] = 0x510e527f;
    buff.h[5] = 0x9b05688c;
    buff.h[6] = 0x1f83d9ab;
    buff.h[7] = 0x5be0cd19;
    buff.data_size = 0;
    buff.chunk_size = 0;
    return buff;
}

static const unsigned int k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define rotate_r(val, bits) (val >> bits | val << (32 - bits))

struct sha256_buff sha256_calc_chunk(struct sha256_buff buff, unsigned char chunk[64]) {
    unsigned int w[64];
    unsigned int tv[8];
    unsigned int i;
    unsigned int pos=0;

    #pragma hls_unroll yes
    for (i=0; i<16; ++i){
        w[i] = (unsigned int) chunk[pos+0] << 24 | (unsigned int) chunk[pos+1] << 16 | (unsigned int) chunk[pos+2] << 8 | (unsigned int) chunk[pos+3];
        pos += 4;
    }
    
    #pragma hls_unroll yes
    for (i=16; i<64; ++i){
        unsigned int s0 = rotate_r(w[i-15], 7) ^ rotate_r(w[i-15], 18) ^ (w[i-15] >> 3);
        unsigned int s1 = rotate_r(w[i-2], 17) ^ rotate_r(w[i-2], 19) ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    // printf("w[0] = %08x\n", w[0]);
    // printf("w[1] = %08x\n", w[1]);
    // printf("w[2] = %08x\n", w[2]);
    // printf("w[3] = %08x\n", w[3]);
    // printf("w[4] = %08x\n", w[4]);
    // printf("w[61] = %08x\n", w[61]);
    // printf("w[62] = %08x\n", w[62]);
    // printf("w[63] = %08x\n", w[63]);
    
    #pragma hls_unroll yes
    for (i = 0; i < 8; ++i)
        tv[i] = buff.h[i];
    
    #pragma hls_unroll yes
    for (i=0; i<64; ++i){
        unsigned int S1 = rotate_r(tv[4], 6) ^ rotate_r(tv[4], 11) ^ rotate_r(tv[4], 25);
        unsigned int ch = (tv[4] & tv[5]) ^ (~tv[4] & tv[6]);
        unsigned int temp1 = tv[7] + S1 + ch + k[i] + w[i];
        unsigned int S0 = rotate_r(tv[0], 2) ^ rotate_r(tv[0], 13) ^ rotate_r(tv[0], 22);
        unsigned int maj = (tv[0] & tv[1]) ^ (tv[0] & tv[2]) ^ (tv[1] & tv[2]);
        unsigned int temp2 = S0 + maj;
        
        tv[7] = tv[6];
        tv[6] = tv[5];
        tv[5] = tv[4];
        tv[4] = tv[3] + temp1;
        tv[3] = tv[2];
        tv[2] = tv[1];
        tv[1] = tv[0];
        tv[0] = temp1 + temp2;
    }

    #pragma hls_unroll yes
    for (i = 0; i < 8; ++i)
        buff.h[i] += tv[i];

    // printf("h[0] = %08x\n", buff.h[0]); // DIFFERENT
    return buff;
}

struct sha256_buff sha256_update(struct sha256_buff buff, unsigned char data[SIZE], int size) {
    int pos = 0;
    // const char* ptr = (const char*)data;
    buff.data_size += size;
    /* If there is data left in buff, concatenate it to process as new chunk */
    // if (size + buff.chunk_size >= 64) {
    //     char tmp_chunk[64];
    //     #pragma hls_unroll yes
    //     for (int i=0; i<buff.chunk_size; i++) {
    // 	    tmp_chunk[i] = buff.last_chunk[i]; 
    //     } 
    //     #pragma hls_unroll yes
    //     for (int i=0; i<64 - buff.chunk_size; i++) {
    // 	    tmp_chunk[buff.chunk_size + i] = data[pos + i]; 
    //     } 
    //     pos += (64 - buff.chunk_size);
    //     size -= (64 - buff.chunk_size);
    //     buff.chunk_size = 0;
    //     buff = sha256_calc_chunk(buff, tmp_chunk);
    // }
    /* Run over data chunks */
    // #pragma hls_unroll yes
    // while (size  >= 64) {
    //     buff = sha256_calc_chunk(buff, ptr);
    //     ptr += 64;
    //     size -= 64; 
    // }
    
    /* Save remaining data in buff, will be reused on next call or finalize */
    #pragma hls_unroll yes
    for (int i=0; i<SIZE; i++) {
         buff.last_chunk[buff.chunk_size + i] = data[pos + i]; 
    } 
    buff.chunk_size += size;

    // printf("c[0] = %d\n", buff.last_chunk[0]);
    // printf("c[1] = %d\n", buff.last_chunk[1]);

    return buff;
}

struct sha256_buff sha256_finalize(struct sha256_buff buff) {
    buff.last_chunk[buff.chunk_size] = 0x80;
    buff.chunk_size++;

    // for (int i=0; i<64 - buff.chunk_size; i++) {
    #pragma hls_unroll yes
    for (int i=0; i<64 - (SIZE + 1); i++) {
        buff.last_chunk[i + buff.chunk_size] = 0; 
    } 

    // printf("c[0] = %d\n", buff.last_chunk[0]);
    // printf("c[1] = %d\n", buff.last_chunk[1]);
    // printf("c[2] = %d\n", buff.last_chunk[2]);
    // printf("c[3] = %d\n", buff.last_chunk[3]);
    // printf("cs = %d\n", buff.chunk_size);

    /* If there isn't enough space to fit int64, pad chunk with zeroes and prepare next chunk */
    // if (buff.chunk_size > 56) {
    //     buff = sha256_calc_chunk(buff, buff.last_chunk);
    //     #pragma hls_unroll yes
    //     for (int i=0; i<64; i++) {
    //         buff.last_chunk[i] = 0; 
    //     } 
    // }

    /* Add total size as big-endian int64 x8 */
    int size = buff.data_size * 8;
    int i;
    #pragma hls_unroll yes
    for (i = 8; i > 0; --i) {
        buff.last_chunk[55+i] = size & 255;
	// printf("c[55+i] = %d\n", buff.last_chunk[55+i]);
        size >>= 8;
    }

    buff = sha256_calc_chunk(buff, buff.last_chunk);

    return buff;
}

struct hash sha256_read(struct sha256_buff buff) {
    int i;
    struct hash h;
    #pragma hls_unroll yes
    for (i = 0; i < 8; i++) {
        h.v[i*4] = (buff.h[i] >> 24) & 255;
        h.v[i*4 + 1] = (buff.h[i] >> 16) & 255;
        h.v[i*4 + 2] = (buff.h[i] >> 8) & 255;
        h.v[i*4 + 3] = buff.h[i] & 255;
    }
    return h;
}

#pragma hls_top
struct hash entrypoint(unsigned char a, unsigned char b) {
    int size = 2;
    unsigned char data[2] = {a, b};
    struct hash h;
    struct sha256_buff buff;

    buff = sha256_init();
    buff = sha256_update(buff, data, size);
    buff = sha256_finalize(buff);
    h = sha256_read(buff);

    return h;
}
