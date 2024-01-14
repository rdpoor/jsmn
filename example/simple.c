/**
 * @file: simple.c
 *
 * To compile, run and clean up:
 *
 * cc simple.c ../jsmn.c -o simple ; ./simple ; rm -f ./simple
 */
#include "../jsmn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * A small example of jsmn parsing when JSON structure is known and number of
 * tokens is predictable.
 */

static const char *JSON_STRING =
    "{\"user\": \"johndoe\", \"admin\": false, \"uid\": 1000,\n  "
    "\"groups\": [\"users\", \"wheel\", \"audio\", \"video\"]}";

static int jsoneq(jsmn_token_t *tok, const char *s) {
    if (jsmn_token_type(tok) == JSMN_STRING && jsmn_token_stringeq(tok, s)) {
        return 0;
    } else {
        return -1;
    }
}

int main() {
    int r;
    jsmn_parser_t p;
    jsmn_token_t t[128]; /* We expect no more than 128 tokens */

    jsmn_init(&p, t, sizeof(t) / sizeof(t[0]));
    r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING));
    if (r < 0) {
        printf("Failed to parse JSON: %d\n", r);
        return 1;
    }

    /* Assume the top-level element is an object */
    if (r < 1 || jsmn_token_type(&t[0]) != JSMN_OBJECT) {
        printf("Object expected\n");
        return 1;
    }

    int i = jsmn_child_of(&p, 0);
    if (i == -1) {
        printf("Could not find object keys\n");
        return 1;
    }

    while (i > 0) {
        jsmn_token_t *key = jsmn_token_ref(&p, i);
        jsmn_token_t *value = jsmn_token_ref(&p, i + 1);

        if (jsoneq(key, "user") == 0) {
            printf("- User: %.*s\n", jsmn_token_strlen(value),
                   jsmn_token_string(value));
        } else if (jsoneq(key, "admin") == 0) {
            printf("- Admin: %.*s\n", jsmn_token_strlen(value),
                   jsmn_token_string(value));
        } else if (jsoneq(key, "uid") == 0) {
            printf("- UID: %.*s\n", jsmn_token_strlen(value),
                   jsmn_token_string(value));
        } else if (jsoneq(key, "groups") == 0) {
            // expect an array of group names
            jsmn_token_t *g;
            int j = jsmn_child_of(&p, i + 1);
            printf("- Groups:\n");
            while ((g = jsmn_token_ref(&p, j)) != NULL) {
                printf("  * %.*s\n", jsmn_token_strlen(g),
                       jsmn_token_string(g));
                j = jsmn_sibling_of(&p, j);
            }
        } else {
            printf("Unexpected key: %.*s\n", jsmn_token_strlen(key),
                   jsmn_token_string(key));
        }
        // advance to next key at this level
        i = jsmn_sibling_of(&p, i + 1);
    }
    return EXIT_SUCCESS;
}
