#ifndef _UTIL_H_
#define _UTIL_H_

#include "BGLib.h"

#define DEBUG
#ifdef DEBUG
#define DBG(...) printf (__VA_ARGS__)
#else
#define DBG(...) (void)0
#endif

int char2int(char input);

void str2mac(const char * src, uint8 * target);

void print_mac(const uint8 * mac);

void phex(uint8_t* str);

int average_array(int nums[10]);

#endif
