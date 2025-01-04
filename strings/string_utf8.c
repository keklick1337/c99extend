/*
 * by Vladislav Tislenko aka keklick1337 (2025)
 * string_utf8.c
 *
 * Implementation for the extended UTF-8 string library in C99.
 * Provides basic dynamic string operations, UTF-8 checks, BOM removal, etc.
 */

#include "string_utf8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * Internal helper: count UTF-8 code points
 */
static size_t utf8_codepoint_count(const char* data, size_t length) {
    size_t count = 0;
    size_t i = 0;
    while (i < length) {
        unsigned char b0 = (unsigned char)data[i];
        if (b0 <= 0x7F) {
            i += 1;
        } else if (b0 >= 0xC2 && b0 <= 0xDF) {
            i += 2;
        } else if (b0 >= 0xE0 && b0 <= 0xEF) {
            i += 3;
        } else if (b0 >= 0xF0 && b0 <= 0xF4) {
            i += 4;
        } else {
            // invalid or out-of-range => break or skip
            break;
        }
        count++;
    }
    return count;
}

/*
 * Create an empty String
 */
String str_init(void) {
    String s;
    s.data        = NULL;
    s.len_bytes   = 0;
    s.len_utf8    = 0;
    s.cap         = 0;
    return s;
}

/*
 * Create a String from a C-string (char*)
 */
String str_from_cstr(const char* cstr) {
    String s = str_init();
    if (!cstr) return s;

    size_t length = strlen(cstr);
    s.cap        = length + 1;
    s.data       = (char*)malloc(s.cap);
    if (s.data) {
        memcpy(s.data, cstr, length + 1);
        s.len_bytes = length;
        s.len_utf8  = utf8_codepoint_count(s.data, s.len_bytes);
    }
    return s;
}

/*
 * Free the string (safe if already NULL)
 */
void str_free(String* s) {
    if (!s) return;
    if (s->data) {
        free(s->data);
        s->data = NULL;
    }
    s->len_bytes = 0;
    s->len_utf8  = 0;
    s->cap       = 0;
}

/*
 * Return pointer to internal buffer (for printf, etc.)
 */
const char* str_data(const String* s) {
    if (!s || !s->data) return "";
    return s->data;
}

/*
 * Reserve more capacity if needed
 */
void str_reserve(String* s, size_t new_cap) {
    if (!s) return;
    if (new_cap > s->cap) {
        char* tmp = (char*)realloc(s->data, new_cap);
        if (tmp) {
            s->data = tmp;
            s->cap  = new_cap;
        }
    }
}

/*
 * Push back a single character (ASCII or extended)
 *
 * NOTE: If you push back a multi-byte character manually,
 * it's up to you to ensure it forms a valid sequence.
 * For single ASCII chars (<= 0x7F), it's obviously 1 code point.
 */
void str_push_back(String* s, char c) {
    if (!s) return;
    if (s->len_bytes + 1 >= s->cap) {
        size_t new_cap = (s->cap == 0) ? 2 : (s->cap * 2);
        str_reserve(s, new_cap);
    }
    if (s->data) {
        s->data[s->len_bytes] = c;
        s->len_bytes++;
        s->data[s->len_bytes] = '\0';
        s->len_utf8 = utf8_codepoint_count(s->data, s->len_bytes);
    }
}

/*
 * Concatenate src onto dest (in-place)
 */
void str_concat(String* dest, const String* src) {
    if (!dest || !src || !src->data) return;
    size_t needed = dest->len_bytes + src->len_bytes + 1; // +1 for '\0'
    if (needed > dest->cap) {
        str_reserve(dest, needed);
    }
    if (dest->data) {
        memcpy(dest->data + dest->len_bytes, src->data, src->len_bytes + 1);
        dest->len_bytes += src->len_bytes;
        dest->len_utf8 = utf8_codepoint_count(dest->data, dest->len_bytes);
    }
}

/*
 * Return a new String as the sum (concatenation) of s1 + s2
 */
String str_plus(const String* s1, const String* s2) {
    if (!s1 || !s1->data) {
        return str_from_cstr(s2 ? s2->data : "");
    }
    if (!s2 || !s2->data) {
        return str_from_cstr(s1->data);
    }

    String result = str_init();
    size_t total_len = s1->len_bytes + s2->len_bytes;
    result.cap = total_len + 1;
    result.data = (char*)malloc(result.cap);
    if (result.data) {
        memcpy(result.data, s1->data, s1->len_bytes);
        memcpy(result.data + s1->len_bytes, s2->data, s2->len_bytes + 1); // +1 for '\0'
        result.len_bytes = total_len;
        result.len_utf8  = utf8_codepoint_count(result.data, result.len_bytes);
    }
    return result;
}

/* ===================================================================
 * UTF-8 validation (RFC 3629)
 * =================================================================== */
bool utf8_validate(const char* data, size_t length) {
    if (!data) return true; // empty or null is "ok"
    size_t i = 0;
    while (i < length) {
        unsigned char b0 = (unsigned char)data[i];
        uint32_t codepoint = 0;
        size_t extraBytes = 0;

        // 1-byte: 0xxxxxxx
        if (b0 <= 0x7F) {
            codepoint = b0;
            extraBytes = 0;
        }
        // 2-byte: 110xxxxx 10xxxxxx
        else if (b0 >= 0xC2 && b0 <= 0xDF) {
            extraBytes = 1;
            codepoint = b0 & 0x1F;
        }
        // 3-byte: 1110xxxx 10xxxxxx 10xxxxxx
        else if (b0 >= 0xE0 && b0 <= 0xEF) {
            extraBytes = 2;
            codepoint = b0 & 0x0F;
        }
        // 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        else if (b0 >= 0xF0 && b0 <= 0xF4) {
            extraBytes = 3;
            codepoint = b0 & 0x07;
        } else {
            // invalid lead byte
            return false;
        }

        // check we have enough continuation bytes
        if (i + extraBytes >= length) {
            return false;
        }

        // read continuation bytes
        for (size_t j = 0; j < extraBytes; j++) {
            unsigned char bx = (unsigned char)data[i + 1 + j];
            if ((bx & 0xC0) != 0x80) {
                return false;
            }
            codepoint = (codepoint << 6) | (bx & 0x3F);
        }

        // check for overlong or invalid codepoints
        switch (extraBytes) {
            case 1:
                if (codepoint < 0x80) return false;
                break;
            case 2:
                if (codepoint < 0x800) return false;
                if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return false; // no surrogates
                break;
            case 3:
                if (codepoint < 0x10000) return false;
                if (codepoint > 0x10FFFF) return false;
                break;
            default:
                break; // 0 => single byte => OK
        }

        i += (extraBytes + 1);
    }
    return true;
}

bool str_validate_utf8(const String* s) {
    if (!s || !s->data) return true;
    return utf8_validate(s->data, s->len_bytes);
}

/*
 * Preflight check: if invalid, print message and return false.
 * Real code might attempt to fix or clear the string.
 */
bool str_preflight_utf8(String* s) {
    if (!s || !s->data) return true;
    bool ok = utf8_validate(s->data, s->len_bytes);
    if (!ok) {
        printf("Preflight failed: string is not valid UTF-8.\n");
    } else {
        printf("Preflight success: string is valid UTF-8.\n");
    }
    return ok;
}

/* ===================================================================
 * BOM support
 * =================================================================== */
/*
 * If the first three bytes are 0xEF, 0xBB, 0xBF, remove them in-place.
 */
bool str_remove_utf8_bom(String* s) {
    if (!s || !s->data || s->len_bytes < 3) return false;
    unsigned char b0 = (unsigned char)s->data[0];
    unsigned char b1 = (unsigned char)s->data[1];
    unsigned char b2 = (unsigned char)s->data[2];
    if (b0 == 0xEF && b1 == 0xBB && b2 == 0xBF) {
        size_t new_len = s->len_bytes - 3;
        memmove(s->data, s->data + 3, new_len);
        s->data[new_len] = '\0';
        s->len_bytes = new_len;
        s->len_utf8  = utf8_codepoint_count(s->data, s->len_bytes);
        return true;
    }
    return false;
}

/* ===================================================================
 * CRLF / LF handling
 * =================================================================== */
void str_strip_crlf(String* s) {
    if (!s || !s->data || s->len_bytes == 0) return;
    while (s->len_bytes > 0) {
        char last_char = s->data[s->len_bytes - 1];
        if (last_char == '\n' || last_char == '\r') {
            s->data[s->len_bytes - 1] = '\0';
            s->len_bytes--;
        } else {
            break;
        }
    }
    s->len_utf8 = utf8_codepoint_count(s->data, s->len_bytes);
}