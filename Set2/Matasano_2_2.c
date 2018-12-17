#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include <math.h>
#include "b64.h"
#include <openssl/aes.h>

/*
    Compile as:
    > gcc Matasano_2_2.c b64.c -lssl -lcrypto -o 2-2
*/

int main(int argc, char* argv[]) {
    char IV[16];
    int i;
    if (argc < 4) {
        printf("Usage: ./2-2 <File name> <Key> <e/d> <IV>\n");
        printf("IV is all 0 bytes if not specified\n");
        printf("e for encrypt, d for decrypt\n");
        exit(1);
    } else if (argc == 4) {
        for(i = 0; i < 16; i++) IV[i] = 0;
    } else {
        for(i = 0; i < 16; i++) IV[i] = argv[4][i];
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
    i = 0;
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

    int j;
    if (*(argv[3]) == 'e') {
        AES_set_encrypt_key(asciiKey, 128, &key);

        // First block is special because it uses IV
        for(j = 0; j < 16; j++) buffer[j] = buffer[j] ^ IV[j];
        AES_encrypt(buffer, output, &key);

        // Now we do the other blocks
        for(i = 1; i < bLen / 16; i++) {
            for(j = 0; j < 16; j++) {
                buffer[16*i + j] = buffer[16*i + j] ^ output[16*(i-1) + j];
            }
            AES_encrypt(buffer + 16*i, output + 16*i, &key);
        }
    } else if (*(argv[3]) == 'd') {
        AES_set_decrypt_key(asciiKey, 128, &key);
        for(i = 0; i < bLen / 16; i++) {
            AES_decrypt(buffer + 16*i, output + 16*i, &key);
        }
        // Just gotta XOR each decrypted block against the previous ciphertext block now
        for(j = 0; j < 16; j++) output[j] = output[j] ^ IV[j];
        for(i = 1; i < bLen / 16; i++) {
            for(j = 0; j < 16; j++) {
                output[16*i + j] = output[16*i + j] ^ buffer[16*(i-1) + j];
            }
        }
    }

    output[bLen] = '\0';
    printf("%s\n", output);
    //for(i = 0; i < bLen; i++) printf("%d\n", output[i]);

    free(buffer);
    free(output);
    return 0;
}
