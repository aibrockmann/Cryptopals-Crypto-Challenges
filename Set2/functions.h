#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// Function that takes a hex digit as a character and produces its value as an int
char hexCharVal(char);

// Function that XORs a single byte against a string
void byteXOR(char*, char, char*);

// Same as byteXOR, but on an input ascii string
void byteXORascii(char*, char, char*, int len);

// Function that determines whether an ascii character is an English letter
int isLetter(char);

// Letter frequency table
float freq[26];

// Repeated key XOR function
void repeatXOR(char*, char*, char*);

// Hamming distance on individual characters
int hamDistChar(char, char);

// Hamming distance between two strings
int hamDist(char*, char*);

// Hamming distance on strings that may contain '\0' characters
int hamDistLen(char*, char*, int);

// Single byte XOR decryption, copied with minimal change from Matasano_1_4.c
char decrAscii(char* ciph, char* buffer, int len);

// PKCS #7 padding function
void PKCS7(char*, int);

// PKCS #7 padding alternate implementation
void PKCS7_2(char*, int, int);

// AES ECB function
void AES_ECB(char*, char*, int, char*, int, int);

// AES CBC function
void AES_CBC(char*, char*, int, char*, int, int, char*);

#endif
