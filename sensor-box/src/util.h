#ifndef _UTIL_H_
#define _UTIL_H_

#define DEBUG
#ifdef DEBUG
#define DBG(...) printf (__VA_ARGS__)
#else
#define DBG(...) (void)0
#endif

#endif
