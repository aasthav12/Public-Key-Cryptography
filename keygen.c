#include "numtheory.h"
#include "randstate.h"
#include "ss.h"

#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

void usage(char *exec) {
    fprintf(stderr,
        "SYNOPSIS\n"
        "   Generates an SS public/private key pair.\n"
        "\n"
        "USAGE\n"
        "   %s [-hv] [-b bits] [-i iters] [-n pbfile.pub] [-d pvfile.priv] [-s seed]\n"
        // https://discord.com/channels/1035678172856995900/1061813507164733460/1077481653443756072 above line from this
        "\n"
        "OPTIONS\n"
        "   -h              Display program help and usage.\n"
        "   -v              Display verbose program output.\n"
        "   -b bits         Minimum bits needed for public key n (default: 256).\n"
        "   -i iterations   Miller-Rabin iterations for testing primes (default: 50).\n"
        "   -n pbfile       Public key file (default: ss.pub).\n"
        "   -d pvfile       Private key file (default: ss.priv).\n"
        "   -s seed         Random seed for testing.\n",
        exec);
}

#define OPTIONS "b:i:n:d:s:vh"

int main(int argc, char **argv) {
    int opt = 0;
    uint64_t bits = 256;
    uint64_t iters = 50;
    uint64_t seed = time(NULL);
    bool verbose_flag = false;
    char *pb_file = "ss.pub";
    char *pv_file = "ss.priv";

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'b': bits = strtoul(optarg, NULL, 10); break;
        case 'i': iters = strtoul(optarg, NULL, 10); break;
        case 'n': pb_file = optarg; break;
        case 'd': pv_file = optarg; break;
        case 's': seed = strtol(optarg, NULL, 10); break;
        case 'v': verbose_flag = true; break;
        case 'h': usage(argv[0]); return 0;
        default: usage(argv[0]); return 0;
        }
    }

    // if iters is negative set it to the default 50
    if ((int) iters < 0) {
        iters = 50;
    }

    // if bits is negative set it to the default time
    if ((int) bits < 0) {
        bits = 256;
    }

    // if iters is negative set it to the default time
    if ((int) seed < 0) {
        seed = time(NULL);
    }

    // check if files are NULL
    if (pb_file == NULL) {
        fprintf(stderr, "ERROR IN OPENING FILE");
        return 1;
    }

    if (pv_file == NULL) {
        fprintf(stderr, "ERROR IN OPENING FILE");
        return 1;
    }

    // Open the public and private key files using fopen()
    FILE *pbfile = fopen(pb_file, "w");
    FILE *pvfile = fopen(pv_file, "w");

    // set file permissions to 0600
    int file_perm = fileno(pvfile);
    fchmod(file_perm, 0600);

    // Initialize the random state
    randstate_init(seed);

    // Make the public key
    mpz_t p, q, n;
    mpz_inits(p, q, n, NULL);
    ss_make_pub(p, q, n, bits, iters);

    // Make the private key
    mpz_t d, pq;
    mpz_inits(d, pq, NULL);
    ss_make_priv(d, pq, p, q);

    // Get the user name as a string
    char *username_file = getenv("USER");

    // Write public key to its file
    ss_write_pub(n, username_file, pbfile);

    // Write private key to its file
    ss_write_priv(pq, d, pvfile);

    // if verbose output is enabled
    if (verbose_flag == true) {
        gmp_printf("user = %s\n", username_file); // username
        gmp_printf("p (%d bits) = %Zd\n", mpz_sizeinbase(p, 2), p); // the first large prime p
        gmp_printf("q (%d bits) = %Zd\n", mpz_sizeinbase(q, 2), q); // the second large prime q
        gmp_printf("n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n); // the public key n
        gmp_printf("d (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d); // the private exponent d
        gmp_printf("pq (%d bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq); // the private modulus pq
    }

    // clear all variables and close all files
    fclose(pbfile);
    fclose(pvfile);
    randstate_clear();
    mpz_clears(p, q, n, d, pq, NULL);

    // terminate the program
    return 0;
}
