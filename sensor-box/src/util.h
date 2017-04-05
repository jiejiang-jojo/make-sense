#ifndef _UTIL_H_
#define _UTIL_H_

#include "inttypes.h"

#define DEBUG
#ifdef DEBUG
#define DBG(...) printf (__VA_ARGS__)
#else
#define DBG(...) (void)0
#endif

int char2int(char input);

void hex2bin(const char* src, uint8_t * target);

#endif
