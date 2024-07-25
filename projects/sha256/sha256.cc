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

#include "sha256.h"

// struct sha256_buff buff;
int  buff_data_size;
int  buff_h[8];
char buff_last_chunk[64];
char buff_chunk_size;

/* Initialization, must be called before any further use */
void sha256_init();

/* Process block of data of arbitary length, can be used on data streams (files, etc) */
void sha256_update(const void* data, int size);

/* Produces final hash values (digest) to be read
   If the buffer is reused later, init must be called again */
void sha256_finalize();

/* Read digest into 32-byte binary array */
void sha256_read(char* hash);

void memcpy(char *dest, const char *src, int n) { 
    // char *csrc = (char *)src; 
    // char *cdest = (char *)dest; 
    // Copy contents of src[] to dest[] 
    #pragma hls_unroll yes
    for (int i=0; i<n; i++) {
        dest[i] = src[i]; 
    } 
}

void memset(char *dest, char ch, int n) { 
    #pragma hls_unroll yes
    for (int i=0; i<n; i++) {
        dest[i] = ch; 
    } 
}

void sha256_init() {
    buff_h[0] = 0x6a09e667;
    buff_h[1] = 0xbb67ae85;
    buff_h[2] = 0x3c6ef372;
    buff_h[3] = 0xa54ff53a;
    buff_h[4] = 0x510e527f;
    buff_h[5] = 0x9b05688c;
    buff_h[6] = 0x1f83d9ab;
    buff_h[7] = 0x5be0cd19;
    buff_data_size = 0;
    buff_chunk_size = 0;
}

static const int k[64] = {
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

static void sha256_calc_chunk(const char* chunk) {
    int w[64];
    int tv[8];
    int i;

    #pragma hls_unroll yes
    for (i=0; i<16; ++i){
        w[i] = (int) chunk[0] << 24 | (int) chunk[1] << 16 | (int) chunk[2] << 8 | (int) chunk[3];
        chunk += 4;
    }
    
    #pragma hls_unroll yes
    for (i=16; i<64; ++i){
        int s0 = rotate_r(w[i-15], 7) ^ rotate_r(w[i-15], 18) ^ (w[i-15] >> 3);
        int s1 = rotate_r(w[i-2], 17) ^ rotate_r(w[i-2], 19) ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    
    #pragma hls_unroll yes
    for (i = 0; i < 8; ++i)
        tv[i] = buff_h[i];
    
    #pragma hls_unroll yes
    for (i=0; i<64; ++i){
        int S1 = rotate_r(tv[4], 6) ^ rotate_r(tv[4], 11) ^ rotate_r(tv[4], 25);
        int ch = (tv[4] & tv[5]) ^ (~tv[4] & tv[6]);
        int temp1 = tv[7] + S1 + ch + k[i] + w[i];
        int S0 = rotate_r(tv[0], 2) ^ rotate_r(tv[0], 13) ^ rotate_r(tv[0], 22);
        int maj = (tv[0] & tv[1]) ^ (tv[0] & tv[2]) ^ (tv[1] & tv[2]);
        int temp2 = S0 + maj;
        
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
        buff_h[i] += tv[i];
}

void sha256_update(const void* data, int size) {
    const char* ptr = (const char*)data;
    buff_data_size += size;
    /* If there is data left in buff, concatenate it to process as new chunk */
    if (size + buff_chunk_size >= 64) {
        char tmp_chunk[64];
        memcpy(tmp_chunk, buff_last_chunk, buff_chunk_size);
        memcpy(tmp_chunk + buff_chunk_size, ptr, 64 - buff_chunk_size);
        ptr += (64 - buff_chunk_size);
        size -= (64 - buff_chunk_size);
        buff_chunk_size = 0;
        sha256_calc_chunk(tmp_chunk);
    }
    /* Run over data chunks */
    #pragma hls_unroll yes
    while (size  >= 64) {
        sha256_calc_chunk(ptr);
        ptr += 64;
        size -= 64; 
    }
    
    /* Save remaining data in buff, will be reused on next call or finalize */
    memcpy(buff_last_chunk + buff_chunk_size, ptr, size);
    buff_chunk_size += size;
}

void sha256_finalize() {
    buff_last_chunk[buff_chunk_size] = 0x80;
    buff_chunk_size++;
    memset(buff_last_chunk + buff_chunk_size, 0, 64 - buff_chunk_size);

    /* If there isn't enough space to fit int64, pad chunk with zeroes and prepare next chunk */
    if (buff_chunk_size > 56) {
        sha256_calc_chunk(buff_last_chunk);
        memset(buff_last_chunk, 0, 64);
    }

    /* Add total size as big-endian int64 x8 */
    int size = buff_data_size * 8;
    int i;
    #pragma hls_unroll yes
    for (i = 8; i > 0; --i) {
        buff_last_chunk[55+i] = size & 255;
        size >>= 8;
    }

    sha256_calc_chunk(buff_last_chunk);
}

void sha256_read(char* hash) {
    int i;
    #pragma hls_unroll yes
    for (i = 0; i < 8; i++) {
        hash[i*4] = (buff_h[i] >> 24) & 255;
        hash[i*4 + 1] = (buff_h[i] >> 16) & 255;
        hash[i*4 + 2] = (buff_h[i] >> 8) & 255;
        hash[i*4 + 3] = buff_h[i] & 255;
    }
}

// static void bin_to_hex(const void* data, int len, char* out) {
//     static const char* const lut = "0123456789abcdef";
//     int i;
//     for (i = 0; i < len; ++i){
//         char c = ((const char*)data)[i];
//         out[i*2] = lut[c >> 4];
//         out[i*2 + 1] = lut[c & 15];
//     }
// }

// void sha256_read_hex(const struct sha256_buff* buff, char* hex) {
//     char hash[32];
//     sha256_read(buff, hash);
//     bin_to_hex(hash, 32, hex);
// }

// void sha256_easy_hash(const void* data, int size, char* hash) {
//     struct sha256_buff buff;
//     sha256_init(&buff);
//     sha256_update(&buff, data, size);
//     sha256_finalize(&buff);
//     sha256_read(&buff, hash);
// }

// void sha256_easy_hash_hex(const void* data, size_t size, char* hex) {
//     char hash[32];
//     sha256_easy_hash(data, size, hash);
//     bin_to_hex(hash, 32, hex);
// }

#pragma hls_top
char entrypoint(char a, char b) {
    int size = 2;
    char data[2] = {a, b};
    char hash[32];
    sha256_init();
    sha256_update(data, size);
    sha256_finalize();
    sha256_read(hash);
    return hash[0];
}
