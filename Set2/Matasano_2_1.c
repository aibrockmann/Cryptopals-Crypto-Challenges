#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.h"
#include <math.h>

/*
    Compile as:
    > gcc Matasano_2_1.c functions.c -lm -o 2-1
*/

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: ./2-1 <Message> <Block size>\n");
        exit(1);
    }

    int blockSize = atoi(argv[2]);
    int len = strlen(argv[1]);

    // Determine total number of blocks so we know how much memory to allocate
    int numBlocks = (int) ceil( (double) len / (double) blockSize );
    char* buffer = (char*) malloc( (numBlocks * blockSize + 1) * sizeof(char) );
    strcpy(buffer, argv[1]);

    PKCS7(buffer, blockSize);
    printf("%s\n", buffer);

    free(buffer);
    return 0;
}
