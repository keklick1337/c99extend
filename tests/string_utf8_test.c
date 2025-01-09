/*
 * by Vladislav Tislenko aka keklick1337 (2025)
 * string_utf8_test.c
 *
 * Test code for the string_utf8 library in C99.
 * Demonstrates reading, validation, BOM handling, CRLF stripping, etc.
 * All comments are in English.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_utf8.h"

/*
 * Helper function: read a line from the file into a String.
 * Also removes CR/LF at the end and optionally removes the UTF-8 BOM.
 */
bool read_line_as_string(FILE* f, String* out, bool removeBOM) {
    if (!f || !out) return false;

    char buffer[1024];
    // Attempt to read one line
    if (!fgets(buffer, sizeof(buffer), f)) {
        return false; // no more lines or an error occurred
    }

    // Create a temporary String from the buffer
    String tmp = STR(buffer);

    // Strip trailing CR or LF (normalize line endings)
    str_strip_crlf(&tmp);

    // Optionally remove BOM
    if (removeBOM) {
        str_remove_utf8_bom(&tmp);
    }

    // Move the temporary String into 'out'
    str_free(out);
    *out = tmp;

    return true;
}

int main(void) {
    /*
     * 1. Simple tests with ASCII strings
     */
    String s1 = STR("Hello, ");
    String s2 = STR("world!");

    printf("s1: '%s' (bytes = %zu, codepoints = %zu)\n",
           str_data(&s1),
           s1.len_bytes,
           s1.len_utf8);

    printf("s2: '%s' (bytes = %zu, codepoints = %zu)\n",
           str_data(&s2),
           s2.len_bytes,
           s2.len_utf8);

    // In-place concatenation: add s2 onto s1
    str_concat(&s1, &s2);
    printf("After concat, s1: '%s' (bytes = %zu, codepoints = %zu)\n",
           str_data(&s1),
           s1.len_bytes,
           s1.len_utf8);

    // Summation of two strings => returns a new String
    String s3 = str_plus(&s1, &s2);
    printf("Sum: s3: '%s' (bytes = %zu, codepoints = %zu)\n",
           str_data(&s3),
           s3.len_bytes,
           s3.len_utf8);

    // Clean up
    str_free(&s1);
    str_free(&s2);
    str_free(&s3);

    /*
     * 2. UTF-8 tests
     */
    // Create a valid UTF-8 string
    String utf8_valid = STR("Привет, мир! 😃");
    str_preflight_utf8(&utf8_valid);
    printf("utf8_valid: '%s' (bytes = %zu, codepoints = %zu)\n",
           str_data(&utf8_valid),
           utf8_valid.len_bytes,
           utf8_valid.len_utf8);

    // Create an invalid UTF-8 string (simulate wrong bytes)
    String utf8_invalid = str_init();
    // Push some invalid leading bytes: 0xFE, 0xAB
    str_push_back(&utf8_invalid, (char)0xFE);
    str_push_back(&utf8_invalid, (char)0xAB);
    // Manually add '\0' at the end
    str_push_back(&utf8_invalid, '\0');
    // Consider only the first 2 bytes as "payload"
    utf8_invalid.len_bytes = 2;

    // Now check validity
    if (!str_preflight_utf8(&utf8_invalid)) {
        printf("We will clear the invalid string.\n");
        str_free(&utf8_invalid);
    }
    str_free(&utf8_valid);

    /*
     * 3. Reading UTF-8 from stdin
     *    (the user can type e.g. "你好" + Enter or any other UTF-8 text)
     */
    printf("Enter some UTF-8 text: ");
    fflush(stdout);

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        // Convert user input to our String type
        String from_stdin = STR(buffer);

        // Strip CRLF if any
        str_strip_crlf(&from_stdin);

        // Remove a BOM if typed (unlikely, but just in case)
        str_remove_utf8_bom(&from_stdin);

        if (str_preflight_utf8(&from_stdin)) {
            printf("You entered valid UTF-8: '%s' (bytes = %zu, codepoints = %zu)\n",
                   str_data(&from_stdin),
                   from_stdin.len_bytes,
                   from_stdin.len_utf8);
        } else {
            printf("Your input is invalid UTF-8.\n");
        }
        str_free(&from_stdin);
    }

    /*
     * 4. Reading from files (test_utf8_nobom.txt and test_utf8_bom.txt)
     */
    printf("\n=== Reading from 'test_files/test_utf8_nobom.txt' ===\n");
    {
        FILE* f = fopen("test_files/test_utf8_nobom.txt", "r");
        if (!f) {
            printf("Cannot open file 'test_files/test_utf8_nobom.txt' for reading.\n");
        } else {
            String line = str_init();
            // Read each line, no BOM removal
            while (read_line_as_string(f, &line, false)) {
                if (str_preflight_utf8(&line)) {
                    printf("[NoBOM File] Valid UTF-8: '%s' (bytes = %zu, codepoints = %zu)\n",
                           str_data(&line),
                           line.len_bytes,
                           line.len_utf8);
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
            // Read each line, remove BOM if found
            while (read_line_as_string(f, &line, true)) {
                if (str_preflight_utf8(&line)) {
                    printf("[BOM File] Valid UTF-8: '%s' (bytes = %zu, codepoints = %zu)\n",
                           str_data(&line),
                           line.len_bytes,
                           line.len_utf8);
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
