#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

/*
    Pretty easy challenge. Really, the hardest part was calling it correctly from the
    command line on the given example text; I initially forgot to put quotes around
    the input string in the terminal. (A problem, given that that example had spaces
    and a \n character.)

    The bulk of the work is in the repeatXOR function, relegated to functions.c since
    I'll be using it elsewhere.
*/

char hexDig[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                 '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: ./1-5 <plaintext> <key>\n");
        exit(1);
    }

    // These lines write the ascii ciphertext into buffer
    int pLen = strlen(argv[1]);
    char* buffer = (char*) malloc( (pLen + 1) * sizeof(char) );
    repeatXOR(argv[1], argv[2], buffer);

    // All that remains is to convert the ascii ciphertext into hex
    char* output = (char*) malloc(2 * pLen + 1);
    int i;
    char c;
    for(i = 0; i < pLen; i++) {
        c = buffer[i];
        output[2*i] = hexDig[c / 16];
        output[2*i + 1] = hexDig[c % 16];
    }

    printf("%s\n", output);
    free(buffer);
    free(output);

    return 0;
}
