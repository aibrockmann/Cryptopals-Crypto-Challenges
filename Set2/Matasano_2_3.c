#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include <math.h>
#include "b64.h"
#include <openssl/aes.h>
#include <time.h>

/*
    Compile as:
    > gcc Matasano_2_3.c functions.c -lssl -lcrypto -lm -o 2-3
*/

// Key generator function that writes a random key into "key"
void keyGen(char* key, int keyLen) {
    int i;
    srand(time(NULL));
    for(i = 0; i < keyLen; i++) {
        key[i] = rand() % 256;
    }
}

// Function that encrypts under ECB or CBC with a random key.
// 5-10 bytes are randomly added before and after the plaintext, and the modified
// plaintext then receives PKCS #7 padding.
// Output buffer should be long enough for appended bytes and padding.
// Return result is the overall padded plaintext length.
int mysteryEnc(char* in, int mLen, char* out) {
    unsigned char* key = (unsigned char*) malloc(17 * sizeof(char));
    keyGen(key, 16);

    // 0 is ECB, and 1 is CBC
    srand(time(NULL));
    int mode = rand() % 2;
    unsigned char IV[16];
    int i;
    if (mode == 1) {
        for(i = 0; i < 16; i++) IV[i] = (rand() % 256);
    }

    // Append amounts
    int prepend = (rand() % 6) + 5;
    int postpend = (rand() % 6) + 5;

    // Perform padding
    int appendLen = prepend + mLen + postpend;
    int padLen = (appendLen / 16 + 1) * 16;
    if (appendLen % 16 == 0) padLen -= 16;
    unsigned char* padIn = (unsigned char*) malloc( (padLen + 1) * sizeof(char) );
    for(i = 0; i < prepend; i++) padIn[i] = '0';  // Append content is unimportant
    for(i = prepend; i < prepend + mLen; i++) padIn[i] = in[i - prepend];
    for(i = prepend + mLen; i < appendLen; i++) padIn[i] = '0';
    PKCS7_2(padIn, appendLen, 16);

    // Perform encryption
    if (mode == 0) {
        AES_ECB(padIn, out, padLen, key, 16, 1);
    } else {
        AES_CBC(padIn, out, padLen, key, 16, 1, IV);
    }

// The following can be safely deleted, though it makes for a nice means of testing
// an otherwise opaque program
/*
    printf("Plaintext:\n");
    for(i = 0; i < mLen; i++) printf("%d ", in[i]);
    printf("\n");
    printf("Mode: %d\n", mode);
    printf("Ciphertext:\n");
    for(i = 0; i < padLen; i++) printf("%d ", out[i]);
    printf("\n");
*/

    free(padIn);
    return padLen;
}

int main(int argc, char* argv[]) {
    // ECB mode can be detected by forcing two identical plaintext blocks.
    // Knowing that 5 to 10 bytes will be prepended, we need a message of at least
    // 43 identical bytes to guarantee this:
    // With 5 prepended bytes, 11 message bytes will finish the first block,
    //   and the other 32 will form identical plaintext blocks.
    // With more prepended bytes, the second and third plaintext blocks will still
    //   be identical (and some of the plaintext will spill over into block four).

    // Create input buffer with plaintext
    unsigned char* in = (unsigned char*) malloc( (43 + 1) * sizeof(char) );
    in[43] = '\0';  // Null terminated for good measure
    int inLen = 43;

    // Output buffer size of 64 is sufficient; plaintext, prepend, and postpend
    // will take at most 63 bytes, so padding will at most fill out the 4th block
    unsigned char* out = (unsigned char*) malloc( (64 + 1) * sizeof(char) );
    out[64] = '\0';  // Null terminator probably not necessary

    // Perform mystery encryption
    int outLen = mysteryEnc(in, inLen, out);

    // Check whether second and third ciphertext blocks are identical
    int i;
    int ECB = 1;
    for(i = 0; i < 16; i++) {
        if (out[16 + i] != out[32 + i]) {
            ECB = 0;
        }
    }

    // Announce result
    if (ECB == 1) {
        printf("ECB\n");
    } else {
        printf("CBC\n");
    }

    free(in);
    free(out);
    return 0;
}
