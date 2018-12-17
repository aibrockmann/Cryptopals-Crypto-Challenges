#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include <math.h>
#include "b64.h"
#include <openssl/aes.h>
#include <time.h>

#define secretStr "Um9sbGluJyBpbiBteSA1LjAKV2l0aCBteSByYWctdG9wIGRvd24gc28gbXkg"\
                  "aGFpciBjYW4gYmxvdwpUaGUgZ2lybGllcyBvbiBzdGFuZGJ5IHdhdmluZyBq"\
                  "dXN0IHRvIHNheSBoaQpEaWQgeW91IHN0b3A/IE5vLCBJIGp1c3QgZHJvdmUg"\
                  "YnkK"
#define secretStrLen 184

/*
    Compile as:
    > gcc Matasano_2_4.c functions.c b64.c -lssl -lcrypto -lm -o 2-4
*/

// Function that encrypts under ECB with a consistent, randomly generated key.
// A consistent secret string is appended to the plaintext, and the plaintext
// is then padded via PKCS#7.
// Output buffer should be long enough for appended bytes and padding.
// Return result is the overall padded plaintext length.
int oracle(char* in, int mLen, char* out) {
    // Yes, I'm writing out the key explicitly here. But it was randomly generated,
    // and I'll be using no knowledge of it in the decryption.
    unsigned char key[] = {246, 65, 236, 90, 94, 116, 117, 186, 41, 76, 236, 211,
                           29, 248, 102, 28};

    // Decode the secret string
    int asciiLen = 3 * secretStrLen / 4;
    char* secretStrAscii = (char*) malloc( (asciiLen + 1) * sizeof(char) );
    b64decode(secretStr, secretStrAscii);

    // Append the secret string to the input text
    int padLen = mLen + asciiLen;
    if (padLen % 16 != 0) padLen += (16 - (padLen % 16));
    char* plaintext = (char*) malloc( (padLen + 1) * sizeof(char) );
    int i;
    for(i = 0; i < mLen; i++) {
        plaintext[i] = in[i];
    }
    for(i = 0; i < asciiLen; i++) {
        plaintext[mLen + i] = secretStrAscii[i];
    }

    // Perform padding
    PKCS7_2(plaintext, padLen, 16);

    // Encrypt the plaintext, writing ciphertext into out
    AES_ECB(plaintext, out, padLen, key, 16, 1);

    free(secretStrAscii);
    free(plaintext);
    return padLen;
}

int main(int argc, char* argv[]) {
    int asciiLen = 3 * secretStrLen / 4;
    unsigned char* decrypted = (unsigned char*) malloc( (asciiLen + 1) * sizeof(char) );

    // Detect block length
    int blockLen;
    int ciphLenFirst = 0, ciphLenCurr = 0;
    int i = 0;
    // 1000 characters should be plenty for determining block length
    unsigned char* in = (unsigned char*) malloc(1000);
    unsigned char* out = (unsigned char*) malloc(1000);
    while(ciphLenFirst == ciphLenCurr) {
        ciphLenCurr = oracle(in, i, out);
        if (i == 0) ciphLenFirst = ciphLenCurr;
        i++;
    }
    blockLen = ciphLenCurr - ciphLenFirst;

    // Detect whether ECB is being used
    int ECB = 1;  // 1 indicates that ECB is in use; 0 indicates otherwise
    for(i = 0; i < 2 * blockLen; i++) {
        in[i] = 'A';
    }
    oracle(in, 2 * blockLen, out);
    for(i = 0; i < blockLen; i++) {
        if (out[i] != out[blockLen + i]) {
            ECB = 0;
        }
    }
    if (ECB != 1) {
        printf("ECB not in use\n");
        exit(1);
    }

    // Determine the plaintext bytes one at a time.
    // The trick is to sequentially feed an input long enough that the first
    // unknown byte appears at the end of a block (i.e., at an index equivalent
    // to -1 modulo blockLen). The unknown byte can then be determined via an
    // oracle dictionary attack.

    // When determining byte i, use plaintext bytes i-15 through i-1, with byte
    // j being set to 'A' if j < 0. Since byte i comes at the end of its block,
    // this gives us the full plaintext block.
    // Append 'A' bytes at the beginning so that byte i is at the end of a block.
    // There are i+1 bytes up through byte i, so add blockLen - (i+1 % blockLen).
    // Then just gotta determine which ciphertext block to look at. Ciphertext
    // block index (from 0) can be found by determining index j of byte i within
    // the padded message; ciphertext block index is j / blockLen. (e.g., with
    // block size 16, index 15 points us to ciphertext block 15 / 16 = 0, and
    // index 47 points us to block 47 / 16 = 2.)
    // Since byte i is only pushed to the end of its block, and never into the
    // next block, we should just be able to compute the ciphertext block index
    // as i / blockLen.
    int j, k, flag, numToAdd, ciphBlockInd;
    int secretChar;
    unsigned char* plainBlock = (unsigned char*) malloc( (blockLen + 1) * sizeof(char) );
    unsigned char* currBlock = (unsigned char*) malloc(1000);
    for(i = 0; i < asciiLen; i++) {
        numToAdd = blockLen - ( (i+1) % blockLen );
        if (numToAdd == blockLen) numToAdd = 0;
        ciphBlockInd = i / blockLen;

        // Construct the plaintext block of the next secret character,
        // with the last character not determined yet
        for(j = i - (blockLen - 1); j < i; j++) {
            if (j < 0) {
                plainBlock[j + (blockLen - 1) - i] = 'A';
            } else {
                plainBlock[j + (blockLen - 1) - i] = decrypted[j];
            }
        }

        // Feed the appropriate input to the oracle
        for(j = 0; j < numToAdd; j++) {
            in[j] = 'A';
        }
        oracle(in, numToAdd, out);

        // Perform dictionary attack
        secretChar = -1;
        for(j = 0; j < 256; j++) {
            flag = 1;
            plainBlock[blockLen - 1] = (char) j;
            oracle(plainBlock, blockLen, currBlock);
            for(k = 0; k < blockLen; k++) {
                if (currBlock[k] != out[ciphBlockInd * blockLen + k]) {
                    flag = 0;
                }
            }
            if (flag == 1) {
                secretChar = j;
                break;
            }
        }

        if (secretChar == -1) {
            // Debugging print statements; could be deleted, since it's working
            // now, but I'm leaving them here. Final problem was that numToAdd
            // could be equal to blockLen, so my code was sometimes adding a
            // full block when it didn't need to add anything.
            /*
            printf("blockLen: %d\n", blockLen);
            printf("i: %d\n", i);
            printf("numToAdd: %d\n", numToAdd);
            printf("ciphBlockInd: %d\n", ciphBlockInd);
            printf("in: %s\n", in);
            printf("plainBlock: %s\n", plainBlock);
            printf("decrypted: %s\n", decrypted);
            */
            printf("No match found\n");
            exit(1);
        } else {
            decrypted[i] = (unsigned char) secretChar;
        }
    }

    printf("%s\n", decrypted);

    // Free allocated memory here
    free(decrypted);
    free(in);
    free(out);
    free(currBlock);
    free(plainBlock);
    return 0;
}
