#include "numtheory.h"
#include "randstate.h"
#include "ss.h"

#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

void usage(char *exec) {
    fprintf(stderr,
        "SYNOPSIS\n"
        "   Decrypts data using SS decryption.\n"
        "   Encrypted data is encrypted by the encrypt program.\n"
        "\n"
        "USAGE\n"
        "   %s [OPTIONS]\n"
        "\n"
        "OPTIONS\n"
        "   -h              Display program help and usage.\n"
        "   -v              Display verbose program output.\n"
        "   -i infile       Input file of data to decrypt (default: stdin).\n"
        "   -o outfile      Output file for decrypted data (default: stdout).\n"
        "   -n pvfile       Private key file (default: ss.priv).\n",
        exec);
}

#define OPTIONS "i:o:n:vh"

int main(int argc, char **argv) {
    int opt = 0;
    FILE *infile = NULL;
    FILE *outfile = NULL;
    FILE *pvfile = fopen("ss.priv", "r");
    bool verbose_flag = false;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w"); break;
        case 'n': pvfile = fopen(optarg, "r"); break;
        case 'v': verbose_flag = true; break;
        case 'h': usage(argv[0]); return 0;
        default: usage(argv[0]); return 0;
        }
    }

    // If no infile is specified set it to stdin
    if (infile == NULL) {
        infile = stdin;
    }

    // If no outfile is specified set it to stdout
    if (outfile == NULL) {
        outfile = stdout;
    }

    // check if private key file can be opened
    if (pvfile == NULL) {
        fprintf(stderr, "ERROR PVFILE CANNOT BE OPENED.\n");
        return 1;
    }

    // read the private key from the opened private key file
    mpz_t pq, d;
    mpz_inits(pq, d, NULL);
    ss_read_priv(pq, d, pvfile);

    // if verbose output is enabled
    if (verbose_flag == true) {
        gmp_printf("pq (%d bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq); // the private modulus pq
        gmp_printf("d (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d); // the private key d
    }

    // decrypt the file
    ss_decrypt_file(infile, outfile, d, pq);

    // clear all variables and close the private key file
    fclose(pvfile);
    mpz_clears(pq, d, NULL);

    // terminate the program
    return 0;
}
