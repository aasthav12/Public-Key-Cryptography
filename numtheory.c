#include <stdio.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdint.h>

#include "numtheory.h"
#include "randstate.h"

// all functions are pseudo code translation from assignment pdf

// computer the greatest common divisor of "a" and "b" and store the results in "g"
void gcd(mpz_t g, const mpz_t a, const mpz_t b) {
    mpz_t a_tmp, b_tmp, t;
    mpz_inits(a_tmp, b_tmp, t, NULL);

    // store a in its tmp variable
    mpz_set(a_tmp, a);
    // store b in its tmp variable
    mpz_set(b_tmp, b);

    while (mpz_sgn(b_tmp) != 0) {
        mpz_set(t, b_tmp); // set t to b_tmp
        mpz_mod(b_tmp, a_tmp, b_tmp); // store the modulus of a and b in b
        mpz_set(a_tmp, t); // set a as t
    }

    // mimic returning a by setting param g as the value of a
    mpz_set(g, a_tmp);

    // clear all mpz variables
    mpz_clears(a_tmp, b_tmp, t, NULL);
}

// computer the inverse "o"  of a modulo "n"
void mod_inverse(mpz_t o, const mpz_t a, const mpz_t n) {
    // initialize all variables
    mpz_t r, r_prime, t, t_prime, q, r_tmp, qr_product, t_tmp, qt_product;
    mpz_inits(r, r_prime, t, t_prime, q, r_tmp, qr_product, t_tmp, qt_product, NULL);

    mpz_set(r, n); // set r equal to n
    mpz_set(r_prime, a); // set r_prime equal to a
    mpz_set_si(t, 0); // set t equal to 0
    mpz_set_si(t_prime, 1); // set t_prime equal to 1

    // while r_prime does not equal 0
    while (mpz_sgn(r_prime) != 0) {
        // floor divide r by r_prime and store in q
        mpz_fdiv_q(q, r, r_prime);
        // store r in a temp for swap
        mpz_set(r_tmp, r);
        // set r equal to r_prime
        mpz_set(r, r_prime);
        // multiply q by r_prime and store in qr_product
        mpz_mul(qr_product, q, r_prime);
        // store r - qr_product in r_prime
        mpz_sub(r_prime, r_tmp, qr_product);

        // store t in a temp for swap
        mpz_set(t_tmp, t);
        // set t equal to t_prime
        mpz_set(t, t_prime);
        // multiply q by t_prime and store in qt_product
        mpz_mul(qt_product, q, t_prime);
        // store t - qt_product in qt_product
        mpz_sub(qt_product, t_tmp, qt_product);
        // set qt_product to t_prime
        mpz_set(t_prime, qt_product);
    }

    // check if r is greater than 1
    if (mpz_cmp_ui(r, 1) > 0) {
        // return no inverse
        // set o to 0
        mpz_set_si(o, 0);
        return;
    }

    // check if t is less than 0
    if (mpz_cmp_ui(t, 0) < 0) {
        // set t equal to t plus n
        mpz_add(t, t, n);
    }

    // mimic returning "t" by setting t to "o"
    mpz_set(o, t);

    // clear all mpz variables
    mpz_clears(r, r_prime, t, t_prime, q, r_tmp, qr_product, t_tmp, qt_product, NULL);
}

// "o" stores the computed result, "a" represents the base raised to the exponent "d" power modulo "n"
// computes fast modular exponentiation
void pow_mod(mpz_t o, const mpz_t a, const mpz_t d, const mpz_t n) {
    // initialize all variables
    mpz_t p, o_tmp, d_tmp, two;
    mpz_inits(p, o_tmp, d_tmp, two, NULL);

    mpz_set(d_tmp, d); // store d as a tmp var

    mpz_set(p, a); // set p to a
    mpz_set_ui(o_tmp, 1); // setting v to 1

    while (mpz_sgn(d_tmp) > 0) {
        // check if d is odd using built in function
        if (mpz_odd_p(d_tmp) != 0) {
            // perform (v * p)
            mpz_mul(o_tmp, o_tmp, p);
            // perform mod n and assign to o_tmp
            mpz_mod(o_tmp, o_tmp, n);
        }

        // perform (p * p)
        mpz_mul(p, p, p);
        // perform mod n and assign to p
        mpz_mod(p, p, n);

        // floor divide d_tmp by 2 and assign to d_tmp
        mpz_fdiv_q_ui(d_tmp, d_tmp, 2);
    }

    // mimic return v by setting value of o_tmp to the parameter o
    mpz_set(o, o_tmp);

    // clear all mpz variables
    mpz_clears(p, o_tmp, d_tmp, two, NULL);
}

// Miller-Rabin test for prime "n" using "iters" number of iterations
bool is_prime(const mpz_t n, uint64_t iters) {
    mpz_t r, s, n_temp, n_min_three, a, y, n_min_one, j, s_min_one, two;
    mpz_inits(r, s, n_temp, n_min_three, a, y, n_min_one, j, s_min_one, two, NULL);

    // number checks derived from Professor Longs example on discord
    // https://discord.com/channels/1035678172856995900/1061813507164733460/1063224264649605120
    if (mpz_cmp_ui(n, 3) == 0) {
        // clears mpz vars
        return true;
    }
    if (mpz_cmp_ui(n, 2) == 0) {
        // clears mpz vars
        return true;
    }
    if (mpz_cmp_ui(n, 1) == 0) {
        // clears mpz vars
        return false;
    }
    if (mpz_cmp_ui(n, 0) == 0) {
        // clears mpz vars
        return false;
    }

    // finding the values of r and s while r is odd
    // inspired from Miles Tutoring Section 2/21/2023
    mpz_sub_ui(n_temp, n, 1);
    mpz_set(r, n_temp);
    while (mpz_even_p(r) != 0) {
        mpz_div_ui(r, r, 2);
        mpz_add_ui(s, s, 1);
    }

    //mpz_sub_ui(n_min_three, n, 3);
    mpz_sub_ui(s_min_one, s, 1);

    mpz_set_ui(two, 2);

    for (uint64_t i = 1; i < iters; i += 1) {
        mpz_sub_ui(n_min_three, n, 3);
        // choosing the random a
        mpz_urandomm(a, state, n_min_three);
        mpz_add_ui(a, a, 2); // allows for the {2,3,...n-2}

        // compute the power mod of a,r,n and store in y
        pow_mod(y, a, r, n);
        // compute and store n - 1
        mpz_sub_ui(n_min_one, n, 1);

        if (mpz_cmp_ui(y, 1) != 0 && mpz_cmp(y, n_min_one) != 0) {
            mpz_set_ui(j, 1); // j <-- 1

            while (mpz_cmp(j, s_min_one) <= 0 && mpz_cmp(y, n_min_one) != 0) {
                pow_mod(y, y, two, n);

                if (mpz_cmp_ui(y, 1) == 0) {
                    mpz_clears(r, s, n_temp, n_min_three, a, y, n_min_one, j, s_min_one, two, NULL);
                    return false;
                }

                mpz_add_ui(j, j, 1); // j <-- j+1
            }

            if (mpz_cmp(y, n_min_one) != 0) {
                mpz_clears(r, s, n_temp, n_min_three, a, y, n_min_one, j, s_min_one, two, NULL);
                return false;
            }
        }
    }

    mpz_clears(r, s, n_temp, n_min_three, a, y, n_min_one, j, s_min_one, two, NULL);
    return true;
}

// Generate a prime number which is to be stored in "p"
void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    mpz_t bits_two, rand;
    mpz_inits(bits_two, rand, NULL);
    bool flag = true; // use as while loop condition when testing whether number is prime or not

    // set the bits to base 2
    mpz_ui_pow_ui(bits_two, 2, bits);

    while (flag) {
        // create the random number from 0 to the bits
        mpz_urandomb(rand, state, bits);
        // add the bits in base 2 to the random number
        mpz_add(rand, rand, bits_two);

        // check if generated rand number is prime
        if (is_prime(rand, iters) == true) {
            // set the rand to p because it is a generated prime
            mpz_set(p, rand);
            // clear all mpz variables
            mpz_clears(bits_two, rand, NULL);
            flag = false;
        }
    }
}
