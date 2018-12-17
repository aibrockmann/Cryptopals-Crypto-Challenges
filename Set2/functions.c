#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <openssl/aes.h>

#define AES_ENC 1
#define AES_DEC 0

// Helper function for 1-1
char hexCharVal(char c) {
    // Takes a hex character (upper or lower case letters both fine)
    // and returns its value as a hexadecimal digit
    if (48 <= c && c <= 57) {
        return c - 48;
    } else if (65 <= c && c <= 70) {
        return c - 55;
    } else if (97 <= c && c <= 102) {
        return c - 87;
    } else {
        return 255;
    }
}

// Single byte XOR function for 1-3 and 1-4
void byteXOR(char* s, char c, char* buffer) {
    // Given hex string s and character c, this function writes an ascii string
    // into buffer by XORing c against each character of s.
    // Since s is a hex string with hex pairs representing individual characters,
    // s is assumed to have even length.
    // Also note that two hex digits correspond to one ascii character, so buffer
    // should have half the length of s.
    int len = (int)strlen(s);
    int i;
    char xorVal;
    for(i = 0; i < len/2; i++) {
        // First determine the character for a given hex digit pair
        xorVal = 16 * hexCharVal(s[2*i]) + hexCharVal(s[2*i + 1]);

        buffer[i] = xorVal ^ c;
    }
}

// Single byte XOR function for an input ascii string
void byteXORascii(char* s, char c, char* buffer, int len) {
    // Given an ascii string s and character c, this function writes an ascii string
    // into buffer by XORing c against each character of s.
    // s length is passed as an argument to allow for \0 characters in s.
    int i;
    for(i = 0; i < len; i++) {
        buffer[i] = s[i] ^ c;
    }
}

// Function that determines whether a character is an English letter
int isLetter(char c) {
    if ( (65 <= c && c <= 90) || (97 <= c && c <= 122) ) {
        return 1;
    } else {
        return 0;
    }
}

// Letter frequency table, as a % (and excluding non-letters)
float freq[26] = {8.12, 1.49, 2.71, 4.32, 12.02, 2.3, 2.03, 5.92, 7.31, 0.1, 0.69, 3.98, 2.61, 6.95, 7.68, 1.82, 0.11, 6.02, 6.28, 9.1, 2.88, 1.11, 2.09, 0.17, 2.11, 0.07};

// Repeated key XOR function for 1-5
void repeatXOR(char* plain, char* key, char* buffer) {
    // Given a plaintext and key (both in ascii), this function computes the repeated
    // key XOR and writes the result into buffer. Buffer should have the same length
    // as plain.
    int pLen = (int)strlen(plain);
    int kLen = (int)strlen(key);
    int i;
    for(i = 0; i < pLen; i++) {
        buffer[i] = plain[i] ^ key[i % kLen];
    }
}

// Hamming distance for two characters
int hamDistChar(char c1, char c2) {
    // The hamming distance is the number of 1s in the XOR of the chars
    char xor = c1 ^ c2;
    int i;
    int dist = 0;
    for(i = 0; i < 8; i++) {
        dist += (xor % 2);
        xor /= 2;
    }
    return dist;
}

// Hamming distance between two equal-length strings
int hamDist(char* s1, char* s2) {
    int dist = 0;
    int i = 0;
    while (1) {
        if (s1[i] == '\0') {
            break;
        }
        dist += hamDistChar(s1[i], s2[i]);
        i++;
    }
    return dist;
}

// Hamming distance with '\0' characters allowed in the strings,
// provided that the string length is also passed in.
// B64 decoded challenge 6 file seems to have null characters,
// throwing things off for the above function...
// For the record, this seems to have done the trick.
int hamDistLen(char* s1, char* s2, int n) {
    int dist = 0;
    int i;
    for(i = 0; i < n; i++) {
        dist += hamDistChar(s1[i], s2[i]);
    }
    return dist;
}

// Single byte XOR decryption, copied with minimal change from Matasano_1_4.c
char decrAscii(char* ciph, char* buffer, int len) {
    // Given an ascii string ciph, finds the most plausible decryption under single 
    // byte XOR, writes the result into buffer, and returns the char key
    
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

        byteXORascii(ciph, (char)i, buffer, len);

        for(k = 0; k < len; k++) {
            if (isLetter(buffer[k]) == 0) {
                nonLetters += 1;
                // Check whether character is a space
                if (buffer[k] == 32) spaces += 1;
                if (buffer[k] == ' ' || buffer[k] == '\n' || buffer[k] == '\'') {
                    // Without this if block, I was counting spaces as non-letters,
                    // which makes the non-letter density very high - words would
                    // need to be at most 3 characters long on average to satisfy
                    // the nonLetters <= len / 4 check, assuming no other non-letters
                    // (such as \n and '). This could be extended to overlook some
                    // other characters too, such as '.' and '!'.
                    // While somewhat inelegant, adding this if block was the last
                    // change needed to make 1-6 work.
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

    byteXORascii(ciph, bestChar, buffer, len);
    return bestChar;
}

// PKCS #7 padding is implemented here so it can be easily called anywhere else.
// Assumption is that there is enough space in memory to add padding directly
// to the end of m, but m should still be null-terminated to start with.
void PKCS7(char* m, int blockSize) {
    int len = strlen(m);
    int amountToAdd = blockSize - (len % blockSize);
    if (amountToAdd == blockSize) amountToAdd = 0;
    int i;
    for(i = 0; i < amountToAdd; i++) {
        m[len + i] = (char) amountToAdd;
    }
    m[len + amountToAdd] = '\0';
}

// Alternate PKCS #7 padding function which allows input to contain \0 characters
// before the end, but which takes the input length as an argument.
void PKCS7_2(char* m, int mLen, int blockSize) {
    int amountToAdd = blockSize - (mLen % blockSize);
    if (amountToAdd == blockSize) amountToAdd = 0;
    int i;
    for(i = 0; i < amountToAdd; i++) {
        m[mLen + i] = (char) amountToAdd;
    }
    m[mLen + amountToAdd] = '\0';
}

// AES ECB encryptor and decryptor. The buffer in has the ascii plaintext, which has
// mLen bytes. The result is written into out. The key length must be given in addition
// to the key to allow \0 characters in the key. (Key length should be in bytes.)
// The mode is AES_ENC (1) for encryption and AES_DEC (0) for decryption.
void AES_ECB(char* in, char* out, int mLen, char* asciiKey, int keyLen, int mode) {
    AES_KEY key;
    int i;
    if (mode == AES_ENC) {
        AES_set_encrypt_key(asciiKey, 8 * keyLen, &key);
        for(i = 0; i < mLen / 16; i++) {
            AES_encrypt(in + 16*i, out + 16*i, &key);
        }
    } else if (mode == AES_DEC) {
        AES_set_decrypt_key(asciiKey, 8 * keyLen, &key);
        for(i = 0; i < mLen / 16; i++) {
            AES_decrypt(in + 16*i, out + 16*i, &key);
        }
    }
}

// AES CBC encryptor and decryptor. Same as AES_ECB, but with an IV too.
void AES_CBC(char* in, char* out, int mLen, char* asciiKey, int keyLen, int mode, char* IV) {
    AES_KEY key;
    int i, j;
    if (mode == AES_ENC) {
        AES_set_encrypt_key(asciiKey, 8 * keyLen, &key);
        for(j = 0; j < 16; j++) in[j] = in[j] ^ IV[j];
        AES_encrypt(in, out, &key);
        for(i = 1; i < mLen / 16; i++) {
            for(j = 0; j < 16; j++) {
                in[16*i + j] = in[16*i + j] ^ out[16*(i-1) + j];
            }
            AES_encrypt(in + 16*i, out + 16*i, &key);
        }
    } else if (mode == AES_DEC) {
        AES_set_decrypt_key(asciiKey, 8 * keyLen, &key);
        for(i = 0; i < mLen / 16; i++) {
            AES_decrypt(in + 16*i, out + 16*i, &key);
        }
        for(j = 0; j < 16; j++) out[j] = out[j] ^ IV[j];
        for(i = 1; i < mLen / 16; i++) {
            for(j = 0; j < 16; j++) {
                out[16*i + j] = out[16*i + j] ^ in[16*(i-1) + j];
            }
        }
    }
} 
