#define NUM_WORDS 1  // Number of 64-bit words for a 256-bit integer

// Function to perform modular subtraction
void mod_sub(long long result[NUM_WORDS], const long long a[NUM_WORDS], const long long b[NUM_WORDS], const long long p[NUM_WORDS]) {
    long long borrow = 0;
    #pragma hls_unroll yes
    for (int i = 0; i < NUM_WORDS; i++) {
        long long temp = a[i] - b[i] - borrow;
        result[i] = temp;
        borrow = (a[i] < b[i] || (borrow && temp > a[i])) ? 1 : 0;
    }
    // if (borrow) {
        long long temp_p[NUM_WORDS];
        #pragma hls_unroll yes
        for (int i = 0; i < NUM_WORDS; i++) {
            temp_p[i] = p[i];
        }
        long long carry = 0;
        #pragma hls_unroll yes
        for (int i = 0; i < NUM_WORDS; i++) {
            long long temp = result[i] + temp_p[i] + carry;
            result[i] = temp;
            carry = (temp < result[i] || (carry && temp == result[i])) ? 1 : 0;
        }
    // }
}

// Function to perform modular reduction
void mod_reduce(long long result[NUM_WORDS], const long long value[NUM_WORDS], const long long p[NUM_WORDS]) {
    long long temp[NUM_WORDS];
    int greater_or_equal;

    #pragma hls_unroll yes
    for (int i = 0; i < NUM_WORDS; i++) {
        result[i] = value[i];
    }

    // while (1) {
        greater_or_equal = 0;
        // #pragma hls_unroll yes
        // for (int i = NUM_WORDS - 1; i >= 0; i--) {
        //     if (result[i] > p[i]) {
        //         greater_or_equal = 1;
        //         break;
        //     } else if (result[i] < p[i]) {
        //         greater_or_equal = 0;
        //         break;
        //     }
        // }
        // if (!greater_or_equal) break;
        mod_sub(result, result, p, p);
    // }
}

// Function to perform modular addition
void mod_add(long long result[NUM_WORDS], const long long a[NUM_WORDS], const long long b[NUM_WORDS], const long long p[NUM_WORDS]) {
    long long carry = 0;
    #pragma hls_unroll yes
    for (int i = 0; i < NUM_WORDS; i++) {
        long long temp = a[i] + b[i] + carry;
        result[i] = temp;
        carry = (temp < a[i] || (carry && temp == a[i])) ? 1 : 0;
    }
    mod_reduce(result, result, p);
}



// Function to perform modular multiplication
void mod_mul(long long result[NUM_WORDS], const long long a[NUM_WORDS], const long long b[NUM_WORDS], const long long p[NUM_WORDS]) {
    long long temp[NUM_WORDS] = {0};
    long long carry[NUM_WORDS] = {0};

    #pragma hls_unroll yes
    for (int i = 0; i < NUM_WORDS; i++) {
        #pragma hls_unroll yes
        for (int j = 0; j < NUM_WORDS; j++) {
            // if (i + j < NUM_WORDS) {
                long long prod = (long long)a[i] * b[j] + carry[i + j];
                carry[i + j] = prod >> 32;
                temp[i + j] += (long long)prod;
            // }
        }
    }

    mod_reduce(result, temp, p);
}



// Function to compute modular inverse (placeholder, needs actual implementation)
void mod_inv(long long result[NUM_WORDS], const long long a[NUM_WORDS], const long long p[NUM_WORDS]) {
    // Placeholder for modular inverse function
    #pragma hls_unroll yes
    for (int i = 0; i < NUM_WORDS; i++) {
        result[i] = a[i];  // Not a correct implementation
    }
}

// Elliptic Curve Point Addition (P = P1 + P2)
void ec_add(long long x3[NUM_WORDS], long long y3[NUM_WORDS], const long long x1[NUM_WORDS], const long long y1[NUM_WORDS], const long long x2[NUM_WORDS], const long long y2[NUM_WORDS], const long long p[NUM_WORDS], const long long a[NUM_WORDS]) {
    long long lambda[NUM_WORDS], num[NUM_WORDS], den[NUM_WORDS];
    long long x3_temp[NUM_WORDS], y3_temp[NUM_WORDS];
    long long temp_x1[NUM_WORDS], temp_y1[NUM_WORDS], temp_x2[NUM_WORDS], temp_y2[NUM_WORDS];
    int is_equal = 1;

    // Check if (x1, y1) == (x2, y2)

    // #pragma hls_unroll yes
    // for (int i = 0; i < NUM_WORDS; i++) {
    //     if (x1[i] != x2[i] || y1[i] != y2[i]) {
    //         is_equal = 0;
    //         break;
    //     }
    // }

    // if (is_equal) {
        // Point Doubling
        mod_mul(num, x1, x1, p);
        mod_mul(num, num, x1, p);
        mod_add(num, num, a, p);
        mod_mul(den, y1, y1, p);
        mod_inv(den, den, p);  // Compute modular inverse of denominator
        mod_mul(lambda, num, den, p);
    // } else {
        // Point Addition
        mod_sub(num, y2, y1, p);
        mod_sub(den, x2, x1, p);
        mod_inv(den, den, p);  // Compute modular inverse of denominator
        mod_mul(lambda, num, den, p);
    // }

    mod_mul(x3_temp, lambda, lambda, p);
    mod_sub(x3, x3_temp, x1, p);
    mod_sub(x3, x3, x2, p);

    mod_sub(temp_x1, x1, x3, p);
    mod_mul(y3_temp, temp_x1, lambda, p);
    mod_sub(y3, y3_temp, y1, p);
}

// Elliptic Curve Point Doubling (P = 2 * P)
void ec_double(long long x2[NUM_WORDS], long long y2[NUM_WORDS], const long long x1[NUM_WORDS], const long long y1[NUM_WORDS], const long long p[NUM_WORDS], const long long a[NUM_WORDS]) {
    long long lambda[NUM_WORDS], num[NUM_WORDS], den[NUM_WORDS];

    mod_mul(num, x1, x1, p);
    mod_mul(num, num, x1, p);
    mod_add(num, num, a, p);

    mod_mul(den, y1, y1, p);
    mod_inv(den, den, p);  // Compute modular inverse of denominator
    mod_mul(lambda, num, den, p);

    // mod_mul(x2, lambda, num, p);
    mod_sub(x2, x2, x1, p);
    mod_sub(x2, x2, x1, p);

    mod_sub(y2, x1, x2, p);
    mod_mul(y2, y2, lambda, p);
    mod_sub(y2, y2, y1, p);
}

// Scalar Multiplication (k * P)
void scalar_mul(long long x_r[NUM_WORDS], long long y_r[NUM_WORDS], const long long k[NUM_WORDS], long long x[NUM_WORDS], long long y[NUM_WORDS], const long long p[NUM_WORDS], const long long a[NUM_WORDS]) {
    long long x_temp[NUM_WORDS], y_temp[NUM_WORDS];
    long long x_r_temp[NUM_WORDS], y_r_temp[NUM_WORDS];
    long long k_temp[NUM_WORDS];
    int bit;

    // Initialize result to the point at infinity (0, 0) and use the base point as the initial point
    #pragma hls_unroll yes
    for (int i = 0; i < NUM_WORDS; i++) {
        x_r[i] = 0;
        y_r[i] = 0;
    }
    x_r[0] = 1;  // Base point for initialization
    y_r[0] = 0;  // Base point for initialization

    #pragma hls_unroll yes
    for (int i = 0; i < NUM_WORDS; i++) {
        k_temp[i] = k[i];
    }

    #pragma hls_unroll yes
    for (int i = 0; i < 5; i++) {
        bit = (k_temp[i / 64] >> (i % 64)) & 1;

        ec_add(x_r_temp, y_r_temp, x_r, y_r, x, y, p, a);
        #pragma hls_unroll yes
        for (int j = 0; j < NUM_WORDS; j++) {
            x_r[j] = x_r_temp[j];
            y_r[j] = y_r_temp[j];
        }

        ec_double(x_temp, y_temp, x, y, p, a);
        #pragma hls_unroll yes
        for (int j = 0; j < NUM_WORDS; j++) {
            x[j] = x_temp[j];
            y[j] = y_temp[j];
        }
    }
}

// Example usage of the functions
#pragma hls_top
int simple_sum(int input) {
    // Example prime and curve parameters
    // long long p[NUM_WORDS] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    // long long a[NUM_WORDS] = {0x0, 0x0, 0x0, 0x0};  // Example parameter a for the curve
    long long p[NUM_WORDS] = {0xFFFFFFFFFFFFFFFF};
    long long a[NUM_WORDS] =  {0x0};  // Example parameter a for the curve

    // Example point (x, y)
    // long long x_input = (long long)input;
    // long long x[NUM_WORDS] = {x_input, x_input, x_input, x_input};
    // long long y[NUM_WORDS] = {0x2, 0x0, 0x0, 0x0};

    // // Scalar
    // long long k[NUM_WORDS] = {0x3, 0x0, 0x0, 0x0};

    long long x_input = (long long)input;
    long long x[NUM_WORDS] = {x_input};
    long long y[NUM_WORDS] = {0x2};

    // Scalar
    long long k[NUM_WORDS] = {0x3};

    // Result point
    long long x_r[NUM_WORDS], y_r[NUM_WORDS];

    // Perform scalar multiplication
    scalar_mul(x_r, y_r, k, x, y, p, a);

    // // Print result
    // printf("x_r: ");
    // for (int i = 0; i < NUM_WORDS; i++) {
    //     printf("%016llx ", x_r[i]);
    // }
    // printf("\n");

    // printf("y_r: ");
    // for (int i = 0; i < NUM_WORDS; i++) {
    //     printf("%016llx ", y_r[i]);
    // }
    // printf("\n");

    return (int)x_r[0];
}