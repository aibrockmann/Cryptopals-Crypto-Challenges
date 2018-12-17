#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Function that provides the numeric value of a Base 64 digit
int b64Val(char c) {
    if (65 <= c && c <= 90) {
        // 'A' is ascii 65
        return c - 65;
    } else if (97 <= c && c <= 122) {
        // 'a' is ascii 97
        return c - 71;
    } else if (48 <= c && c <= 57) {
        // '0' is ascii 48
        return c + 4;
    } else if (c == '+') {
        return 62;
    } else if (c == '/') {
        return 63;
    } else {
        // Error; character not base 64
        return -1;
    }
}

// Base 64 decode function. Awkwardly, the challenges so far only ask for Base 64
// encoding... but up through challenge 6, only Base 64 DEcoding is necessary.
void b64decode(char* s, char* buffer) {
    // s should be base 64 encoded with padding, hence its length is a multiple of 4.
    // buffer should have length equal to 3/4 that of s, since each B64 quadruple
    // decodes to 3 bytes (with the possible exception of the last quadruple).
    int i = 0;
    int flag = 0;  // Flag used to determine if the last block has been reached
    int blockVal;
    while (flag == 0) {
        // First produce the numeric value of the base 64 block
        // If one of the last two characters is '=', then this is the last block
        blockVal = 64 * b64Val(s[4*i]) + b64Val(s[4*i + 1]);
        if (s[4*i + 2] == '=') {
            // Final two characters are padding
            flag = 1;
            blockVal = (blockVal >> 4);
            buffer[3*i] = (char) blockVal;
            buffer[3*i + 1] = '\0';
        } else if (s[4*i + 3] == '=') {
            // Final character is padding
            flag = 1;
            blockVal = (64 * blockVal + b64Val(s[4*i + 2])) >> 2;
            buffer[3*i + 1] = (char) (blockVal % 256);
            buffer[3*i] = (char) (blockVal >> 8);
            buffer[3*i + 2] = '\0';
        } else {
            // No padding
            blockVal = 64 * blockVal + b64Val(s[4*i + 2]);
            blockVal = 64 * blockVal + b64Val(s[4*i + 3]);
            buffer[3*i + 2] = (char) blockVal % 256;
            buffer[3*i + 1] = (char) (blockVal >> 8) % 256;
            buffer[3*i] = (char) (blockVal >> 16);

            // This can be the final block even if there's no padding
            if (s[4*i + 4] == '\0') {
                flag = 1;
                buffer[3*i + 3] = '\0';
            }
        }

        i += 1;
    }
}
