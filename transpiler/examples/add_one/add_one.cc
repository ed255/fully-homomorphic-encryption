#include "transpiler/examples/add_one/add_one.h"
// char add_one(char x) { return x + 1; }
#include <stdio.h>
#include <gmp.h>

#pragma hls_top
int add_one() {
    mpz_t prime, result;
    mpz_init(result);
    // Initialize a 256-bit integer
    mpz_init(prime);

    // Set the value of the integer to a 256-bit prime
    mpz_set_str(prime, "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f", 16);

    mpz_mod(prime, prime , result);

    return 0;
}