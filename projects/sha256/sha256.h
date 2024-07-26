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

#ifndef SHA256_H
#define SHA256_H

// #include <stdint.h>
// #include <string.h>
// #include <stddef.h>

// /* Initialization, must be called before any further use */
// void sha256_init(struct sha256_buff* buff);
// 
// /* Process block of data of arbitary length, can be used on data streams (files, etc) */
// void sha256_update(struct sha256_buff* buff, const void* data, size_t size);
// 
// /* Produces final hash values (digest) to be read
//    If the buffer is reused later, init must be called again */
// void sha256_finalize(struct sha256_buff* buff);
// 
// /* Read digest into 32-byte binary array */
// void sha256_read(const struct sha256_buff* buff, char* hash);

struct sha256_buff {
    int data_size;
    int h[8];
    unsigned char last_chunk[64];
    unsigned char chunk_size;

    sha256_buff& operator=(const sha256_buff& b) {
	data_size = b.data_size;
	chunk_size = b.chunk_size;
        #pragma hls_unroll yes
        for(int i=0;i<8;i++) {
            h[i] = b.h[i];
        }      
        #pragma hls_unroll yes
        for(int i=0;i<64;i++) {
            last_chunk[i] = b.last_chunk[i];
        }      
        return *this;
    }
};

struct hash {
    unsigned char v[32];

    hash& operator=(const hash& h) {
        #pragma hls_unroll yes
        for(int i=0;i<32;i++) {
            v[i] = h.v[i];
        }      
        return *this;
    }
};


struct hash entrypoint(unsigned char a, unsigned char b);

// /* Read digest into 64-char string as hex (without null-byte) */
// void sha256_read_hex(const struct sha256_buff* buff, char* hex);
// 
// /* Hashes single contiguous block of data and reads digest into 32-byte binary array */
// void sha256_easy_hash(const void* data, size_t size, char* hash);
// 
// /* Hashes single contiguous block of data and reads digest into 64-char string (without null-byte) */
// void sha256_easy_hash_hex(const void* data, size_t size, char* hex);

#endif
