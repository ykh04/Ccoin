//2022 ykh04
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h> /*dll link*/
#include <time.h>
#include <unistd.h>

void RSAGen(mpz_t *publickey, mpz_t *privatekey, mpz_t *keyCase);
void primeGen(mpz_t *prime, unsigned int bitSize, int isExistMax, mpz_t Max);

void RSAGen(mpz_t *publickey, mpz_t *privatekey, mpz_t *keyCase)
{
    int isPlus;
    mpz_t   p, q, Max, gcdi, dummy;
    mpz_init2(p, 256);
    mpz_init2(q, 256);
    mpz_init2(Max, 512);
    mpz_init2(gcdi, 512);
    mpz_init2(dummy, 512);

    mpz_set_str(gcdi, "1", 16);

    primeGen((mpz_t *)&p, 256, 0, p/*“K“–*/);
    primeGen((mpz_t *)&q, 256, 0, p/*“K“–*/);
    mpz_mul(*keyCase, p, q);
    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);
    mpz_mul(Max, p, q);

reCalcurate:

    primeGen(publickey, 512, 1, Max);
    for (;;) {
        mpz_gcdext(gcdi, *privatekey, dummy, *publickey, Max);
        isPlus = mpz_sgn(*privatekey);
        if (isPlus == 1) {
            break;
        }
        else if (isPlus <= 0) {
            goto reCalcurate;
        }
    }

    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(Max);
    mpz_clear(gcdi);
    mpz_clear(dummy);
    return;
}

void primeGen(mpz_t *prime, unsigned int bitSize, int isExistMax, mpz_t Max)
{
    int param = 0, cParam, counter;

    gmp_randstate_t state;
    gmp_randinit_default(state);

    time_t resetTime;

RESET:
    time(&resetTime);
    gmp_randseed_ui(state, resetTime);
    counter = 0;

    for (;;) {
        mpz_urandomb(*prime, state, bitSize);
        param = mpz_probab_prime_p(*prime, 25);
        if (param == 1) {
            if (isExistMax == 0) {
                break;
            }
            cParam = mpz_cmp(Max, *prime);
            if (cParam > 0) {
                break;
            }
            counter++;
        }
        if (counter == 25) {
            sleep(0.8);
            goto RESET;
        }
    }
    gmp_randclear(state);
    sleep(1);
    return;
}


