#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include <math.h>

// Line length is kept constant at 60 in this challenge
#define LINE_LENGTH 60

/*
    Note: While the challenge says that the file consists of 60-character lines,
    one line only has 58. I arbitrarily filled out that line with ff.

    The primary exercise here was properly reading in individual lines. My approach
    here seems 'dumb' in that it reads in individual characters and passes over
    the \r and \n characters; that is, I kind of implemented getline from scratch.
    But while I saw nice examples online using getline, there can apparently be
    trouble across different libc versions, and I'd need to use the _GNU_SOURCE
    definition that has caused me mild trouble in the past.

    I copied the code from 1-3 and modified the main function to serve as a helper
    function instead. There may have been a more elegant approach, e.g. by shifting
    some stuff to functions.c.

    Also, I went and tested my old python code for this challenge, and it runs
    a LOT slower. This one is instant, while the python implementation takes
    more than a few seconds. I clearly didn't have efficiency on the mind back then.

    Compile as follows:
    > gcc Matasano_1_4.c functions.c -lm -o 1-4
*/

float decrStr(char* ciph, char* buffer) {
    // Given a hex string ciph, finds the most plausible decryption under single 
    // byte XOR, writes the result into buffer, and returns the score
    int len = LINE_LENGTH / 2;
    
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

        byteXOR(ciph, (char)i, buffer);

        for(k = 0; k < len; k++) {
            if (isLetter(buffer[k]) == 0) {
                nonLetters += 1;
                // Check whether character is a space
                if (buffer[k] == 32) spaces += 1;
                if (buffer[k] == ' ' || buffer[k] == '\n' || buffer[k] == '\'') {
                    nonLetters -= 1;
                }
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

    byteXOR(ciph, bestChar, buffer);

    return bestScore;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("An input file name is needed\n");
        exit(1);
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Failed to open file\n");
        exit(1);
    }

    // Arbitrary huge score; if a sensible decryption exists, its score should be lower
    float bestScore = 1000000;
    // String storing the current best decryption
    char bestDecr[LINE_LENGTH / 2 + 1];
    bestDecr[LINE_LENGTH / 2] = '\0';
    // String storing the ciphertext of bestDecr
    char bestCiph[LINE_LENGTH + 1];
    bestCiph[LINE_LENGTH] = '\0';
    // Loop that reads in individual lines and determines best decryption for each.
    // Current best decryption and score are updated appropriately throughout.
    float currScore;
    char currDecr[LINE_LENGTH / 2 + 1];
    currDecr[LINE_LENGTH / 2] = '\0';
    char currCiph[LINE_LENGTH + 1];
    currCiph[LINE_LENGTH] = '\0';
    char c = getc(fp);
    int i;
    while (c != EOF) {
        // Write the next ciphertext line into currCiph.
        // Assumption is that file format is valid; this may hang otherwise.
        currCiph[0] = c;
        for(i = 1; i < LINE_LENGTH; i++) {
            c = getc(fp);
            currCiph[i] = c;
        }

        currScore = decrStr(currCiph, currDecr);
        if (currScore < bestScore) {
            bestScore = currScore;
            strcpy(bestDecr, currDecr);
            strcpy(bestCiph, currCiph);
        }

        c = getc(fp);  // Eat up the \r char, or reach the EOF
        // If we're not at EOF, then each line also has a \n after the \r
        if (c != EOF) {
            c = getc(fp);
            c = getc(fp);
        }
    }

    printf("Most plausible decryption was with the line:\n  %s\n", bestCiph);
    printf("The best decryption was:\n  %s\n", bestDecr);

    fclose(fp);
    return 0;
}
