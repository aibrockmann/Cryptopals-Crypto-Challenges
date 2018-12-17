#ifndef B64_H
#define B64_H

// Function that provides the numeric value of a Base 64 digit
int b64Val(char);

// Base 64 decode function. Awkwardly, the challenges so far only ask for Base 64
// encoding... but up through challenge 6, only Base 64 DEcoding is necessary.
void b64decode(char*, char*);

#endif
