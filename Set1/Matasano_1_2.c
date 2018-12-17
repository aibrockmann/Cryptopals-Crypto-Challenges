#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

char hexDig[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                 '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void hexStrXOR(char* s1, char* s2, char* buffer) {
    // Given hex strings s1 and s2, writes the bitwise XOR of the underlying hex
    // values into buffer. s1 and s2 are assumed to have the same length. If s1 
    // is shorter, s2 is effectively truncated in the lower order bits. If s1 is
    // longer, bad things may happen.
    int len = (int)strlen(s1);
    int i, xorVal;
    for(i = 0; i < len; i++) {
        xorVal = hexCharVal(s1[i]) ^ hexCharVal(s2[i]);
        buffer[i] = hexDig[xorVal];
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Too few arguments; two hex strings are needed\n");
        exit(1);
    }

    char* s1 = argv[1];
    char* s2 = argv[2];
    int len = (int)strlen(s1);
    char* buffer = (char*)malloc( (len + 1) * sizeof(char) );
    hexStrXOR(s1, s2, buffer);
    printf("%s\n", buffer);
    free(buffer);
    
    return 0;
}
