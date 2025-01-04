/*
 * by Vladislav Tislenko aka keklick1337 (2025)
 * string_utf8_test.c
 *
 * Test code for the string_utf8 library.
 * Demonstrates reading, validation, BOM handling, CRLF stripping, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_utf8.h"

/*
 * Helper: read a line from file into a String, removing CR/LF and optionally BOM.
 */
bool read_line_as_string(FILE* f, String* out, bool removeBOM) {
    if (!f || !out) return false;

    char buffer[1024];
    if (!fgets(buffer, sizeof(buffer), f)) {
        return false; // no more lines or error
    }

    String tmp = STR(buffer);
    str_strip_crlf(&tmp);

    if (removeBOM) {
        str_remove_utf8_bom(&tmp);
    }

    // Move result into *out
    str_free(out);
    *out = tmp;

    return true;
}

int main(void) {
    /*
     * 1. Simple tests: normal ASCII strings
     */
    String s1 = STR("Hello, ");
    String s2 = STR("world!");
    printf("s1: '%s' (length: %zu)\n", str_data(&s1), s1.len);
    printf("s2: '%s' (length: %zu)\n", str_data(&s2), s2.len);

    // Concat s2 onto s1 in-place
    str_concat(&s1, &s2);
    printf("After concat: s1: '%s' (length: %zu)\n", str_data(&s1), s1.len);

    // Summation of two strings => new string
    String s3 = str_plus(&s1, &s2);
    printf("Sum: s3: '%s' (length: %zu)\n", str_data(&s3), s3.len);

    // Cleanup
    str_free(&s1);
    str_free(&s2);
    str_free(&s3);

    /*
     * 2. UTF-8 tests
     */
    // Valid UTF-8
    String utf8_valid = STR("Привет, мир! 😃");
    str_preflight_utf8(&utf8_valid);
    printf("utf8_valid: '%s'\n", str_data(&utf8_valid));

    // Invalid UTF-8 (simulate wrong bytes)
    String utf8_invalid = str_init();
    // Put some invalid leading byte
    str_push_back(&utf8_invalid, (char)0xFE);
    str_push_back(&utf8_invalid, (char)0xAB);
    // end with \0
    str_push_back(&utf8_invalid, '\0');
    utf8_invalid.len = 2; // we consider only 2 invalid bytes

    if (!str_preflight_utf8(&utf8_invalid)) {
        printf("We will clear the invalid string.\n");
        str_free(&utf8_invalid);
    }
    str_free(&utf8_valid);

    /*
     * 3. Reading UTF-8 from terminal (stdin)
     *    (User can type UTF-8 text, e.g. "你好" + Enter, or "хуй" + Enter)
     */
    printf("Enter some UTF-8 text: ");
    fflush(stdout);

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        // Create from user input
        String from_stdin = STR(buffer);
        // Strip trailing CRLF
        str_strip_crlf(&from_stdin);
        // Remove BOM if user typed it (unlikely, but just a demonstration)
        str_remove_utf8_bom(&from_stdin);

        if (str_preflight_utf8(&from_stdin)) {
            printf("You entered valid UTF-8: '%s'\n", str_data(&from_stdin));
        } else {
            printf("Your input is invalid UTF-8.\n");
        }
        str_free(&from_stdin);
    }

    /*
     * 4. Reading from files (test_utf8_nobom.txt, test_utf8_bom.txt)
     */
    printf("\n=== Reading from 'test_files/test_utf8_nobom.txt' ===\n");
    {
        FILE* f = fopen("test_files/test_utf8_nobom.txt", "r");
        if (!f) {
            printf("Cannot open file 'test_files/test_utf8_nobom.txt' for reading.\n");
        } else {
            String line = str_init();
            while (read_line_as_string(f, &line, false /* no BOM removal */)) {
                if (str_preflight_utf8(&line)) {
                    printf("[NoBOM File] Valid UTF-8: '%s'\n", str_data(&line));
                } else {
                    printf("[NoBOM File] Invalid UTF-8 found.\n");
                }
            }
            str_free(&line);
            fclose(f);
        }
    }

    printf("\n=== Reading from 'test_files/test_utf8_bom.txt' ===\n");
    {
        FILE* f = fopen("test_files/test_utf8_bom.txt", "r");
        if (!f) {
            printf("Cannot open file 'test_files/test_utf8_bom.txt' for reading.\n");
        } else {
            String line = str_init();
            while (read_line_as_string(f, &line, true /* remove BOM if found */)) {
                if (str_preflight_utf8(&line)) {
                    printf("[BOM File] Valid UTF-8: '%s'\n", str_data(&line));
                } else {
                    printf("[BOM File] Invalid UTF-8 found.\n");
                }
            }
            str_free(&line);
            fclose(f);
        }
    }

    return 0;
}