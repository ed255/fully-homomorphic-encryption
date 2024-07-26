#include <stdio.h>
#include "sha256.h"

int main() {
	unsigned char a = 1;
	unsigned char b = 2;
	struct hash h;

	h = entrypoint(a, b);

	printf("Hash: ");
	for (int i=0; i<32; i++) {
		printf("%02x", h.v[i]);
	}
	printf("\n");
	
	return 0;
}
