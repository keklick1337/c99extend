/*
 * by Vladislav Tislenko aka keklick1337 (2025)
 * string_utf8.h
 *
 * Header file for the extended UTF-8 string library in C99.
 * It declares our String structure and all related functions.
 */

#ifndef K_STRING_UTF8_H
#define K_STRING_UTF8_H

#include <stddef.h> // size_t
#include <stdbool.h> // bool

/*
 * String structure
 */
typedef struct {
    char*  data;        // Pointer to character buffer
    size_t len_bytes;   // Current length in bytes (excluding '\0')
    size_t len_utf8;    // Current length in UTF-8 code points
    size_t cap;         // Allocated capacity (including '\0')
} String;

/*
 * Macros
 */
#define STR(cstr) str_from_cstr((cstr))

/*
 * Basic string functions
 */
String      str_init(void);
String      str_from_cstr(const char* cstr);
void        str_free(String* s);
const char* str_data(const String* s);
void        str_push_back(String* s, char c);
void        str_concat(String* dest, const String* src);
String      str_plus(const String* s1, const String* s2);
void        str_reserve(String* s, size_t new_cap);

/*
 * UTF-8 validation
 */
bool utf8_validate(const char* data, size_t length);
bool str_validate_utf8(const String* s);
bool str_preflight_utf8(String* s);

/*
 * BOM-related
 */
bool str_remove_utf8_bom(String* s);

/*
 * CRLF / LF handling
 */
void str_strip_crlf(String* s);

#endif // K_STRING_UTF8_H
