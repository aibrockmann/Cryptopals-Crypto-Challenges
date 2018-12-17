#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include <math.h>

/*
    This one took quite a lot more than "a few minutes", and while my approach
    is possibly not ideal, I do think this challenge should take some time.
    After challenge 2, performing an XOR on a hex string is easy enough. But
    it's also necessary to somehow score decrypted strings on their general
    resemblance to English, and doing this properly takes some time and effort.

    Compile as follows to avoid problems with the math library and pow function:
    > gcc Matasano_1_3.c functions.c -lm -o 1-3
*/

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Too few arguments; a hex string is needed\n");
        exit(1);
    }

    int len = ( (int)strlen(argv[1]) ) / 2;
    char* buffer = (char*)malloc( (len + 1) * sizeof(char) );

    // Scores for all possible single-byte keys; lower score is better fit
    float scores[256];

    // nonLetters counts the non-letter characters for a particular decryption;
    // counts, similarly, tallies the count for each letter (case insensitive)
    int i, j, k, nonLetters, spaces;
    int counts[26];
    float score;
    // Attempt decryption with each possible character
    for(i = 0; i < 256; i++) {
        nonLetters = 0;
        spaces = 0;
        score = 0;
        for(j = 0; j < 26; j++) counts[j] = 0;

        byteXOR(argv[1], (char)i, buffer);

        for(k = 0; k < len; k++) {
            if (isLetter(buffer[k]) == 0) {
                nonLetters += 1;
                // Check whether character is a space
                if (buffer[k] == 32) spaces += 1;
            } else if (buffer[k] <= 90) {
                // 'A' is ascii 65
                counts[buffer[k] - 65] += 1;
            } else {
                // 'a' is ascii 97
                counts[buffer[k] - 97] += 1;
            }
        }

        // Too many non-letters will be viewed as disqualifying and result in
        // a huge score. Numeric characters are more or less neglected for now,
        // even though intelligible writing can have a high numeric density.
        // After an initial failure, I'm looking for a minimum space rate too.
        // (Success!)
        if ( (nonLetters > len / 4) || (spaces < len / 8) ) {
            score = 1000000;
        } else {
            for(j = 0; j < 26; j++) {
                score += pow(freq[j] - 100 * (float)counts[j] / (float)len, 2);
            }
        }

        scores[i] = score;
    }

    // Determine which character yielded the lowest score
    char bestChar;
    // Simply initializing this variable to a huge value isn't elegant,
    // but it should work
    float bestScore = 1000000;
    for(i = 0; i < 256; i++) {
        if (scores[i] < bestScore) {
            bestChar = (char)i;
            bestScore = scores[i];
        }
    }

    byteXOR(argv[1], bestChar, buffer);
    printf("%s\n", buffer);
    free(buffer);

    return 0;
}
