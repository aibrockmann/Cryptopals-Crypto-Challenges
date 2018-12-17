#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include "b64.h"
#include <math.h>

#define MIN_KEYSIZE 2
#define MAX_KEYSIZE 40

/*
    Compile as:
    gcc Matasano_1_6.c functions.c b64.c -lm -o 1-6

    The challenge description lays out the methodology used here, so there isn't
    much to say about that. I had to fix a minor issue with some code copied from
    1-4, which caused the single byte XOR decryption to only reliably work when
    the plaintext had a very short average word length.

    The range of key sizes tried can be changed by modifying the values of
    MIN_KEYSIZE and MAX_KEYSIZE. I used the 2 through 40 range suggested by the
    challenge text.
*/

// Helper function that determines the most probable key size by computing the
// average, normalized Hamming distance for all possible key sizes
int findKeySize(char* m) {
    // We'll compare 10 different chunks at a time, so set initial distance equal
    // to 10 times the maximum byte distance + 1
    float bestDist = 81.0;
    int bestSize = 0;
    int keySize;
    int i, j;
    float avgDist;
    char* chunks[5];
    for(keySize = MIN_KEYSIZE; keySize <= MAX_KEYSIZE; keySize++) {
        for(i = 0; i < 5; i++) {
            chunks[i] = (char*) malloc(MAX_KEYSIZE * sizeof(char));
            chunks[i][keySize] = '\0';
            for(j = 0; j < keySize; j++) chunks[i][j] = m[keySize * i + j];
        }
        avgDist = 0.0;
        for(i = 0; i < 5; i++) {
            for(j = i+1; j < 5; j++) {
                avgDist += hamDistLen(chunks[i], chunks[j], keySize);
            }
        }
        avgDist /= ((float) keySize);

        if (avgDist < bestDist) {
            bestDist = avgDist;
            bestSize = keySize;
        }
    }
    for(i = 0; i < 5; i++) free(chunks[i]);

    return bestSize;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: ./1-6 <File name>\n");
        exit(1);
    }

    // We'll first read in the file, ignoring all characters that aren't base 64 digits
    // or padding '=' characters. In other words, pass over \r, \n, etc.
    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Failed to open file\n");
        exit(1);
    }
    // Determine file length so we have a close upper bound on how much memory is needed
    // (Assumption is that the file isn't THAT long - say, a couple GB at most)
    fseek(fp, 0L, SEEK_END);
    int fLen = (int) ftell(fp);
    rewind(fp);
    char* f = (char*) malloc( (fLen + 1) * sizeof(char) );

    // Copy file into the allocated memory
    char c;
    int i = 0;
    while (1) {
        c = getc(fp);
        if (c == EOF) {
            f[i] = '\0';
            break;
        } else if ( (b64Val(c) != -1) || (c == '=') ) {
            f[i] = c;
            i++;
        }
    }

    // Decode the base 64-encoded file into a buffer
    int bLen = 3*i / 4;  // One b64 quadruple encodes 3 bytes
    if (f[i-2] == '=') {
        bLen -= 2;  // Final quadruple only produces one ascii char
    } else if (f[i-1] == '=') {
        bLen -= 1;  // Final quadruple produces two ascii chars
    }
    char* buffer = (char*) malloc( (bLen + 1) * sizeof(char) );
    b64decode(f, buffer);
    free(f);  // Raw file content not needed anymore

    // Determine most probable key size based on Hamming distances. In general,
    // it might be more thorough to keep track of the best several lengths
    // instead of just one.
    int keySize = findKeySize(buffer);

    // The next step is to decrypt the ciphertext as a bunch of smaller texts
    // encrypted with single byte XOR. We first need to know how long the smaller
    // texts will be.
    int subTextLen = (int) ceil( (double) bLen / (double) keySize );

    // Create buffers to hold the final decrypted text, the individual single-byte
    // encrypted plaintexts, their individual decryptions, and the key
    char* decrypt = (char*) malloc( (bLen + 1) * sizeof(char) );
    char* subText = (char*) malloc( (subTextLen + 1) * sizeof(char) );
    char* subDecr = (char*) malloc( (subTextLen + 1) * sizeof(char) );
    char* key = (char*) malloc( (keySize + 1) * sizeof(char) );
    key[keySize] = '\0';

    int j, k;
    for(i = 0; i < keySize; i++) {
        // Copy the appropriate sub-chunk into subText
        k = 0;
        for(j = i; j < bLen; j += keySize) {
            subText[k] = buffer[j];
            k++;

        }

        // Perform single byte XOR decryption on the subtext
        key[i] = decrAscii(subText, subDecr, k);

        // Copy the decrypted subtext into the appropriate indices of decrypt
        for(j = 0; j < k; j++) {
            decrypt[i + j * keySize] = subDecr[j];
        }
    }
    printf("Key was:\n%s\n\n", key);
    printf("The plaintext is:\n\n%s", decrypt);

    fclose(fp);
    free(buffer);
    free(key);
    free(subDecr);
    free(subText);
    free(decrypt);
    return 0;
}
