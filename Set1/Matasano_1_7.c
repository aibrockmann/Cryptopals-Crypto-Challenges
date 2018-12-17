#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "b64.h"
#include <openssl/aes.h>

/*
    Compile as:
    > gcc Matasano_1_7.c b64.c -lssl -lcrypto -o 1-7
*/

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: ./1-7 <File name> <Key>\n");
        exit(1);
    }

    // Input file should be base 64 encoded.
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
    unsigned char* buffer = (unsigned char*) malloc( (bLen + 1) * sizeof(char) );
    b64decode(f, buffer);
    free(f);  // Raw file content not needed anymore

    unsigned char* output = (unsigned char*) malloc( (bLen + 1) * sizeof(char) );
    unsigned char asciiKey[16];
    for(i = 0; i < 16; i++) asciiKey[i] = argv[2][i];
    AES_KEY key;
    AES_set_decrypt_key(asciiKey, 128, &key);
    for(i = 0; i < bLen / 16; i++) {
        AES_decrypt(buffer + 16*i, output + 16*i, &key);
    }
    // For some reason, the following line only correctly decrypted the first block
    // with what appeared to be a repeating and seemingly insignificant pattern
    // afterward. Manually decrypting one block at a time worked.
    //AES_ecb_encrypt(buffer, output, (const AES_KEY*) &key, AES_DECRYPT);

    output[bLen] = '\0';
    printf("%s\n", output);

    free(buffer);
    free(output);
    return 0;
}
