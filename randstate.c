#include <stdio.h>
#include <gmp.h>
#include <stdint.h>
#include <stdlib.h>

#include "randstate.h"

gmp_randstate_t state;

// Initialize the global random state "state" with the Mersenne Twister algorithm
void randstate_init(uint64_t seed) {
    srandom(seed);
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, seed);
}

// Free and clear up memory allocated by initializing the global random state
void randstate_clear(void) {
    gmp_randclear(state);
}
