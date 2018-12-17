#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include <math.h>

/*
    Compile as:
    > gcc Matasano_1_8.c functions.c -lm -o 1-8

    Not much to say; just look for a hex string with a repeating 16-byte
    (32 hex character) block.
*/

// Helper function that returns 1 iff b1 and b2 are identical in their first
// n characters
int compareBlocks(char* b1, char* b2, int n) {
    int i;
    for(i = 0; i < n; i++) {
        if (b1[i] != b2[i]) return 0;
    }
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: ./1-8 <File name>\n");
        exit(1);
    }

    // Input file is hex encoded. No decoding necessary; two 16-byte blocks are
    // identical if and only if their 32-character hex encodings are too.
    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Failed to open file\n");
        exit(1);
    }

    // Determine the length of each hex string. It is assumed that all are the
    // same length, and separated by non-hex characters.
    int hexStrLen = 0;
    char c = '0';
    while (1) {
        c = getc(fp);
        if (hexCharVal(c) < 0 || hexCharVal(c) > 15) break;
        hexStrLen++;
    }
    rewind(fp);

    // Strings will be read one-by-one into memory and checked for matching blocks
    char* currStr = (char*) malloc( (hexStrLen + 1) * sizeof(char) );
    currStr[hexStrLen] = '\0';
    int numBlocks = hexStrLen / 32;

    int i, j, k;
    int atEnd = 0;
    c = getc(fp);
    while(!atEnd) {
        for(i = 0; i < hexStrLen; i++) {
            currStr[i] = c;
            c = getc(fp);
        }

        // Pairwise comparison of the blocks comprising currStr
        for(j = 0; j < numBlocks; j++) {
            for(k = j + 1; k < numBlocks; k++) {
                if (compareBlocks(currStr + 32*j, currStr + 32*k, 32) == 1) {
                    printf("%s\n", currStr);
                    j = numBlocks;
                    k = numBlocks;  // Double loop break
                }
            }
        }

        // Pass over non-hex characters to get to the next hex string, and detect
        // the end of the file if it occurs
        while (hexCharVal(c) < 0 || hexCharVal(c) > 15) {
            c = getc(fp);
            if (c == EOF) {
                atEnd = 1;
                break;
            }
        }    
    }

    free(currStr);
    return 0;
}
