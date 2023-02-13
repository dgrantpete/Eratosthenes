#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define BITFIELD_BIT CHAR_BIT
#define to_abs_index(i) (i.byte_index * BITFIELD_BIT + i.bit_index)

typedef unsigned char bitfield_data;

typedef struct {
    int byte_index;
    int bit_index;
} bitfield_index;

void clear_bit(bitfield_data* bitfield_data, bitfield_index* i) {
    bitfield_data[i->byte_index] &= ~(1 << i->bit_index);
}

int next_factor(const bitfield_data* bitfield, int bitfield_len, int abs_index) {
    int byte_index = abs_index / BITFIELD_BIT;
    int bit_index = abs_index % BITFIELD_BIT;

    // Special logic to check first byte starting from start bit (anything preceding)
    bitfield_data x = bitfield[byte_index];
    for (bit_index++; bit_index < BITFIELD_BIT; bit_index++) {
        if (x & (1 << bit_index)) {
            return byte_index * BITFIELD_BIT + bit_index;
        }
    }

    // Check remaining bytes
    for (byte_index++; byte_index < bitfield_len; byte_index++) {
        x = bitfield[byte_index];
        // If `x` is 0, we don't need to iterate through it (no bits are 1)
        if (!x) {
            continue;
        }
        for (bit_index = 0; bit_index < BITFIELD_BIT; bit_index++) {
            if (x & (1 << bit_index)) {
                return byte_index * BITFIELD_BIT + bit_index;
            }
        }
    }

    return -1;
}

void print_bitfield_indexes(const bitfield_data* bitfield, int bitfield_len) {
    bitfield_index i;
    for (i.byte_index = 0; i.byte_index < bitfield_len; i.byte_index++) {
        bitfield_data x = bitfield[i.byte_index];
        for (i.bit_index = 0; i.bit_index < BITFIELD_BIT; i.bit_index++) {
            if (x & (1 << i.bit_index)) {
                printf("%d ", to_abs_index(i));
            }
        }
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Please provide a single argument (an integer between 2 and %d), %d arguments passed.\n", INT_MAX - 1, argc - 1);
        return -1;
    }

    int max_val = atoi(argv[1]);

    if (max_val < 2 || max_val == INT_MAX) {
        printf("'%s' could not be converted to an integer between 2 and %d.\n", argv[1], INT_MAX - 1);
        return -1;
    }

    int bitfield_len = (max_val / BITFIELD_BIT) + 1;

    bitfield_data* primes_bitfield = malloc(sizeof(bitfield_data) * bitfield_len);

    if (primes_bitfield == NULL) {
        puts("Memory allocation for primes bitfield failed.");
        return -1;
    }

    for (size_t i = 0; i < bitfield_len; i++) {
        primes_bitfield[i] = ~0;
    }

    // Turning off the bits which come after `max_val`, we don't want to display them
    for (bitfield_index i = {bitfield_len - 1, max_val % BITFIELD_BIT + 1}; i.bit_index < BITFIELD_BIT; i.bit_index++) {
        clear_bit(primes_bitfield, &i);
    }

    // Clearing the "0" and "1" indexes of the bitfield since neither of them are prime.
    primes_bitfield[0] &= ~0b11;

    bitfield_index i;

    for (int factor = 2, fac_sq = 4; fac_sq < max_val; factor = next_factor(primes_bitfield, bitfield_len, factor), fac_sq = factor * factor) {
        // The div/mod of `factor` and `BITFIELD_BIT` is computed and stored so expensive repeated calls aren't needed
        // to calculate the bit/byte index at each location.
        int byte_change = factor / BITFIELD_BIT;
        int bit_change = factor % BITFIELD_BIT;

        i.byte_index = fac_sq / BITFIELD_BIT;
        i.bit_index = fac_sq % BITFIELD_BIT;

        while (i.byte_index < bitfield_len) {
            if (i.bit_index >= BITFIELD_BIT) {
                i.bit_index -= BITFIELD_BIT;
                i.byte_index += 1;
            } else {
                clear_bit(primes_bitfield, &i);

                i.byte_index += byte_change;
                i.bit_index += bit_change;
            }
        }
    }

    print_bitfield_indexes(primes_bitfield, bitfield_len);

    free(primes_bitfield);
}