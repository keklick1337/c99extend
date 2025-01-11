/*
 * string_utils.c
 *
 * Implementation of the string utility functions declared in string_utils.h.
 * 
 * by Vladislav Tislenko aka keklick1337 (2025)
 */

#include "string_utils.h"
#include <stdlib.h>  /* for malloc, free */
#include <string.h>  /* for strlen, memcpy */

/*
 * adv_strdup
 */
char* adv_strdup(const char* src) {
    if (!src) {
        return NULL;
    }
    /* measure length + 1 for '\0' */
    size_t len = strlen(src) + 1;
    char* dup = (char*)malloc(len);
    if (dup) {
        memcpy(dup, src, len);
    }
    return dup;
}

/*
 * adv_strndup
 */
char* adv_strndup(const char* src, size_t n) {
    if (!src) {
        return NULL;
    }
    /* measure length but limit to 'n' */
    size_t srclen = strlen(src);
    if (srclen > n) {
        srclen = n; /* we only copy up to n */
    }
    /* +1 for '\0' */
    char* dup = (char*)malloc(srclen + 1);
    if (!dup) {
        return NULL;
    }
    memcpy(dup, src, srclen);
    dup[srclen] = '\0';
    return dup;
}

/*
 * adv_strreverse
 */
void adv_strreverse(char* s) {
    if (!s) return;
    /* find length */
    size_t len = strlen(s);
    size_t i = 0, j = len ? (len - 1) : 0;
    while (i < j) {
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        i++;
        j--;
    }
}
