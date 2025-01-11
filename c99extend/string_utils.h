/*
 * string_utils.h
 *
 * A small collection of string utility functions in pure C99.
 * Provides equivalents of strdup, strndup, plus an example strreverse.
 * 
 * by Vladislav Tislenko aka keklick1337 (2025)
 */

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h> /* for size_t */

/*
 * adv_strdup:
 *   Returns a newly allocated copy of 'src'.
 *   The caller must free the returned pointer.
 *   Returns NULL if allocation fails or src is NULL.
 */
char* adv_strdup(const char* src);

/*
 * adv_strndup:
 *   Similar to adv_strdup, but copies at most 'n' characters.
 *   Always appends a '\0', so the resulting string is null-terminated.
 *   Returns NULL on allocation failure or if src is NULL.
 */
char* adv_strndup(const char* src, size_t n);

/*
 * adv_strreverse:
 *   In-place reverse of the string 's' (must be modifiable).
 *   Does nothing if 's' is NULL.
 */
void adv_strreverse(char* s);

#endif /* STRING_UTILS_H */
