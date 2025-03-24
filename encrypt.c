#include "numtheory.h"
#include "randstate.h"
#include "ss.h"

#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#define OPTIONS "i:o:n:vh"

void usage(char *exec) {
    fprintf(stderr,
        "SYNOPSIS\n"
        "   Encrypts data using SS encryption.\n"
        "   Encrypted data is decrypted by the decrypt program.\n"
        "\n"
        "USAGE\n"
        "   %s [OPTIONS]\n"
        "\n"
        "OPTIONS\n"
        "   -h              Display program help and usage.\n"
        "   -v              Display verbose program output.\n"
        "   -i infile       Input file of data to encrypt (default: stdin).\n"
        "   -o outfile      Output file for encrypted data (default: stdout).\n"
        "   -n pbfile       Public key file (default: ss.pub).\n",
        exec);
}

int main(int argc, char **argv) {
    int opt = 0;
    FILE *infile = NULL;
    FILE *outfile = NULL;
    FILE *pbfile = fopen("ss.pub", "r");
    bool verbose_flag = false;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o': outfile = fopen(optarg, "w"); break;
        case 'n': pbfile = fopen(optarg, "r"); break;
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

    // check if public key file can be opened
    if (pbfile == NULL) {
        fprintf(stderr, "ERROR PBFILE CANNOT BE OPENED.\n");
        return 1;
    }

    // Read the public key file from opened public key file
    mpz_t n;
    mpz_init(n);
    // initialize the username
    char username_read[256] = "";

    ss_read_pub(n, username_read, pbfile);

    // if verbose output is enabled
    if (verbose_flag == true) {
        gmp_printf("user = %s\n", username_read); // username
        gmp_printf("n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n); // the public key n
    }

    // encrypt the file
    ss_encrypt_file(infile, outfile, n);

    // clear all variables and close all files
    fclose(infile);
    fclose(outfile);
    fclose(pbfile);
    mpz_clear(n);

    // terminate the program
    return 0;
}
