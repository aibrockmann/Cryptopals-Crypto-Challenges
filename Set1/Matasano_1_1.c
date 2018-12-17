#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
    This might be a lot more complicated than what the challenge authors had in mind;
    I couldn't quite tell how much work was actually expected on the coder's part.
    Given that the set 1 challenges are mostly intended to take a few minutes each,
    it was probably expected that we'd use some existing libraries to do most of the
    work. Still, I thought it'd be an interesting exercise to implement this from
    scratch.

    A more optimized implementation probably would not calculate the number of full
    blocks and leftover characters ahead of time, and would instead complete the
    encoding in one pass (looking at most three characters ahead at a time).

    This implementation works for the challenge example, as well as a handful of
    examples from Wikipedia and a few from other sites.
*/

char b64[64] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

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
        printf("Input is not a hexadecimal digit\n");
        return 255;
    }
}

int main(int argc, char* argv[]) {
    // Input is mostly assumed valid: just one argument, a hex string of even length
    // We will check that we get this one necessary argument, though
    if (argc < 2) {
        printf("No input string given\n");
        exit(1);
    }

    char* hex_string = argv[1];
    int len = (int)strlen(hex_string);

    // Six hex characters are sequentially converted into four base64 characters;
    // last block might be different if input length is not a multiple of 6
    int fullBlocks = len / 6;
    int leftover = len % 6;
    int numBlocks = fullBlocks;
    if (leftover > 0) {
        numBlocks += 1;
    }
    // Each block in the b64 encoded string will consist of 4 characters
    // (I'm being sloppy for now and not checking whether malloc succeeds)
    char* encoded = (char*)malloc( (numBlocks * 4 + 1) * sizeof(char));

    int i, j, blockVal;
    int b64_0, b64_1, b64_2, b64_3;

    // Process the full blocks
    for(i = 0; i < fullBlocks; i++) {
        blockVal = 0;
        for(j = 0; j < 6; j++) {
            blockVal += (int)hexCharVal(hex_string[6*i + j]) * (1 << 4*(5 - j));
        }
        b64_0 = blockVal % 64;
        blockVal /= 64;
        b64_1 = blockVal % 64;
        blockVal /= 64;
        b64_2 = blockVal % 64;
        blockVal /= 64;
        b64_3 = blockVal % 64;

        encoded[4*i] = b64[b64_3];
        encoded[4*i + 1] = b64[b64_2];
        encoded[4*i + 2] = b64[b64_1];
        encoded[4*i + 3] = b64[b64_0];
    }

    // Process last block if we're one hex pair short of a full block
    if (leftover == 4) {
        blockVal = 0;
        for(j = 0; j < 4; j++) {
            blockVal += ( (int)hexCharVal(hex_string[6*i + j]) * (1 << 4*(3-j)) ) << 2;
        }
        b64_1 = blockVal % 64;
        blockVal /= 64;
        b64_2 = blockVal % 64;
        blockVal /= 64;
        b64_3 = blockVal % 64;

        encoded[4*i] = b64[b64_3];
        encoded[4*i + 1] = b64[b64_2];
        encoded[4*i + 2] = b64[b64_1];
        encoded[4*i + 3] = '=';
    }

    // Process last block if we're two hex pairs short of a full block
    if (leftover == 2) {
        blockVal = 0;
        for(j = 0; j < 2; j++) {
            blockVal += ( (int)hexCharVal(hex_string[6*i + j]) * (1 << 4*(1-j)) ) << 4;
        }
        b64_2 = blockVal % 64;
        blockVal /= 64;
        b64_3 = blockVal % 64;

        encoded[4*i] = b64[b64_3];
        encoded[4*i + 1] = b64[b64_2];
        encoded[4*i + 2] = '=';
        encoded[4*i + 3] = '=';
    }

    printf("%s\n", encoded);
    free(encoded);

    return 0;
}
