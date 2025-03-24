#include <stdio.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "numtheory.h"
#include "randstate.h"
#include "ss.h"

// lcm function created using https://discord.com/channels/1035678172856995900/1061813507164733460/1077811448538992750 equation: lcm(a, b)=|ab|/gcd(a,b)
// where a = (p-1) and b = (q-1)
void lcm(mpz_t s, mpz_t a, mpz_t b) {
    // create all necessary mpz variables
    mpz_t mul_ab, pos_numerator, num, denom, divs;
    mpz_inits(mul_ab, pos_numerator, num, denom, divs, NULL);

    // create the numerator which is absolute value of a * b
    mpz_mul(mul_ab, a, b);
    mpz_abs(pos_numerator, mul_ab);
    mpz_set(num, pos_numerator);

    // solve for the denominator which is the gcd of a and b
    gcd(denom, a, b);
    mpz_fdiv_q(divs, num, denom);

    // set the division of num and denom to the variable s
    mpz_set(s, divs);

    // clear all variables
    mpz_clears(mul_ab, pos_numerator, num, denom, divs, NULL);
}

// Creates parts of a new SS public key: two large primes p and q, and n computed as p∗p∗q
void ss_make_pub(mpz_t p, mpz_t q, mpz_t n, uint64_t nbits, uint64_t iters) {
    // initialize all mpz variables
    mpz_t p_minus_one, q_minus_one, p_squared;
    mpz_inits(p_minus_one, q_minus_one, p_squared, NULL);
    // create the range of bits to input in p and q
    uint64_t pbits = random() % ((2 * nbits) / 5 + 1 - (nbits / 5)) + (nbits / 5);
    uint64_t qbits = nbits - pbits;

    // create variables for p-1 and q-1
    mpz_sub_ui(p_minus_one, p, 1);
    mpz_sub_ui(q_minus_one, q, 1);

    // check divisibility, if so you must generate new primes
    make_prime(p, pbits, iters);
    make_prime(q, qbits, iters);

    while (mpz_divisible_p(p, q_minus_one) == 0 || mpz_divisible_p(q, p_minus_one) == 0) {
        make_prime(p, pbits, iters);
        make_prime(q, qbits, iters);

        mpz_sub_ui(p_minus_one, p, 1);
        mpz_sub_ui(q_minus_one, q, 1);
    }

    // update value "n" as p*p*q
    mpz_mul(p_squared, p, p);
    mpz_mul(n, p_squared, q);

    // clear all mpz_variables
    mpz_clears(p_minus_one, q_minus_one, p_squared, NULL);
}

// Creates a new SS private key d given primes p and q and the public key n
void ss_make_priv(mpz_t d, mpz_t pq, const mpz_t p, const mpz_t q) {
    // initialize all mpz variables
    mpz_t p_squared, n, p_minus_one, q_minus_one, lambda_pq;
    mpz_inits(p_squared, n, p_minus_one, q_minus_one, lambda_pq, NULL);

    // calculate n again, as p*p*q
    mpz_mul(p_squared, p, p);
    mpz_mul(n, p_squared, q);

    // calculate the lcm of pq as lcm(p-1,q-1)
    mpz_sub_ui(p_minus_one, p, 1);
    mpz_sub_ui(q_minus_one, q, 1);
    lcm(lambda_pq, p_minus_one, q_minus_one);

    // provide the private modulus and exponent
    mpz_mul(pq, p, q);
    mod_inverse(d, n, lambda_pq);

    // clear all variables
    mpz_clears(p_squared, n, p_minus_one, q_minus_one, lambda_pq, NULL);
}

// Writes a public SS key to pbfile
void ss_write_pub(const mpz_t n, const char username[], FILE *pbfile) {
    mpz_t n_tmp;
    mpz_init(n_tmp);

    mpz_set(n_tmp, n);

    gmp_fprintf(pbfile, "%Zx\n", n_tmp);
    gmp_fprintf(pbfile, "%s\n", username);

    mpz_clear(n_tmp);
}

// Writes a private SS key to pvfile
void ss_write_priv(const mpz_t pq, const mpz_t d, FILE *pvfile) {
    mpz_t pq_tmp, d_tmp;
    mpz_inits(pq_tmp, d_tmp, NULL);

    mpz_set(pq_tmp, pq);
    mpz_set(d_tmp, d);

    gmp_fprintf(pvfile, "%Zx\n", pq_tmp);
    gmp_fprintf(pvfile, "%Zx\n", d_tmp);

    mpz_clears(pq_tmp, d_tmp, NULL);
}

// Reads a public SS key from pbfile
void ss_read_pub(mpz_t n, char username[], FILE *pbfile) {
    gmp_fscanf(pbfile, "%Zx\n", n);
    gmp_fscanf(pbfile, "%s\n", username);
}

// Reads a private SS key from pvfile
void ss_read_priv(mpz_t pq, mpz_t d, FILE *pvfile) {
    gmp_fscanf(pvfile, "%Zx\n", pq);
    gmp_fscanf(pvfile, "%Zx\n", d);
}

// performs SS encryption using formula E(m) = c = m^n (mod n)
void ss_encrypt(mpz_t c, const mpz_t m, const mpz_t n) {
    pow_mod(c, m, n, n);
}

void ss_encrypt_file(FILE *infile, FILE *outfile, const mpz_t n) {
    // calculate the block size k
    size_t k = (mpz_sizeinbase(n, 2) / 2 - 1) / 8;

    // allocate an array that can hold k bytes of type uint8_t *
    uint8_t *arr_block = (uint8_t *) calloc(k, sizeof(uint8_t));

    // set the zeroth byte of the block to 0xFF
    arr_block[0] = 0xFF;

    // initialize mpz variables to be used for encryting
    mpz_t c, m;
    mpz_inits(c, m, NULL);

    // check if end of file has been reached
    if (infile != NULL) {
        size_t j;

        // read at most k-1 bytes from infile and place read bytes into allocated block starting from array 1
        while ((j = fread(arr_block + 1, sizeof(uint8_t), k - 1, infile)) > 0) {
            mpz_import(m, j + 1, 1, sizeof(arr_block[0]), 1, 0,
                arr_block); // 1=most significant word first, 1=endian, and 0=nails
            ss_encrypt(c, m, n);
            // write the encrypted number to outfile
            gmp_fprintf(outfile, "%Zx\n", c);
        }
    }

    // clear all variables and free the array created
    mpz_clears(c, m, NULL);
    free(arr_block);
}

// performs SS decryption using the formula s D(c) = m = c^d (mod pq)
void ss_decrypt(mpz_t m, const mpz_t c, const mpz_t d, const mpz_t pq) {
    pow_mod(m, c, d, pq);
}

void ss_decrypt_file(FILE *infile, FILE *outfile, const mpz_t d, const mpz_t pq) {
    // initialize all mpz variables
    mpz_t c, m;
    mpz_inits(c, m, NULL);

    // solve for k
    size_t k = (mpz_sizeinbase(pq, 2) - 1) / 8;

    // Dynamically allocate an array that can hold k bytes
    uint8_t *arr_block = (uint8_t *) calloc(k, sizeof(uint8_t));

    size_t j;
    while (gmp_fscanf(infile, "%Zx\n", c) != EOF) {
        // decrypt c back to its original value m
        ss_decrypt(m, c, d, pq);

        // convert m back into bytes storing in allocated block

        mpz_export(arr_block, &j, 1, sizeof(uint8_t), 1, 0, m);

        // write j-1 from array of blocks starting from index 1
        fwrite(&arr_block[1], sizeof(uint8_t), j - 1, outfile);
    }

    // clear all variables and free the array created
    free(arr_block);
    mpz_clears(c, m, NULL);
}
