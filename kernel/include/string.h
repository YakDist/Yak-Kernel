#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
#define _STRING_RESTRICT restrict
#else
#define _STRING_RESTRICT
#endif

void *memcpy(void *_STRING_RESTRICT dest, const void *_STRING_RESTRICT src,
             size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

#undef _STRING_RESTRICT

#ifdef __cplusplus
}
#endif

#endif
