/*
 * MIT License
 *
 * Copyright (c) 2010 Serge Zaitsev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "jsmn.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// *****************************************************************************
// local types and definitions

#define START_TO_STR(js, start) (&js[(start)])
#define STR_TO_START(js, str) ((str)-js)

// *****************************************************************************
// forward references to local functions

/**
 * Reset the parser, free and clear all tokens.
 */
static void reset_parser(jsmn_parser_t *parser);

/**
 * Allocates a fresh unused token from the token pool.
 */
static jsmn_token_t *jsmn_alloc_token(jsmn_parser_t *parser);

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token(jsmn_token_t *token, const jsmn_token_type_t type,
                            const char *start, int length);

/**
 * Fills next available token with JSON primitive.
 */
static int jsmn_parse_primitive(jsmn_parser_t *parser, const char *js,
                                const size_t len);

/**
 * Fills next token with JSON string.
 */
static int jsmn_parse_string(jsmn_parser_t *parser, const char *js,
                             const size_t len);

// *****************************************************************************
// public functions

/**
 * Creates a new parser based over a given buffer with an array of tokens
 * available.
 */
void jsmn_init(jsmn_parser_t *parser, jsmn_token_t *tokens,
               unsigned int num_tokens) {
    parser->tokens = tokens;
    parser->num_tokens = num_tokens;
}

/**
 * Parse JSON string and fill tokens.
 */
int jsmn_parse(jsmn_parser_t *parser, const char *js, const size_t len) {
    int r;
    int i;
    int count;
    jsmn_token_t *token;

    reset_parser(parser);
    count = parser->token_count;

    for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
        char c;
        jsmn_token_type_t type;

        c = js[parser->pos];
        switch (c) {
        case '{':
        case '[':
            count++;
            if (parser->tokens == NULL) {
                break;
            }
            token = jsmn_alloc_token(parser);
            parser->level += 1;
            if (token == NULL) {
                return JSMN_ERROR_NOMEM;
            }
            if (parser->parent_index != -1) {
                jsmn_token_t *parent = &parser->tokens[parser->parent_index];
#ifdef JSMN_STRICT
                /* In strict mode an object or array can't become a key */
                if (parent->type == JSMN_OBJECT) {
                    return JSMN_ERROR_INVAL;
                }
#endif
                parent->child_count++;
#ifdef JSMN_PARENT_LINKS
                token->parent_index = parser->parent_index;
#endif
            }
            token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
            token->start = &js[parser->pos];
            parser->parent_index = parser->token_count - 1;
            break;
        case '}':
        case ']':
            parser->level -= 1;
            if (parser->tokens == NULL) {
                break;
            }
            type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
            if (parser->token_count < 1) {
                return JSMN_ERROR_INVAL;
            }
            token = &parser->tokens[parser->token_count - 1];
            for (;;) {
                if (token->start != NULL && token->strlen == -1) {
                    if (token->type != type) {
                        return JSMN_ERROR_INVAL;
                    }
                    token->strlen = (parser->pos + 1) - (token->start - js);
                    parser->parent_index = token->parent_index;
                    break;
                }
                if (token->parent_index == -1) {
                    if (token->type != type || parser->parent_index == -1) {
                        return JSMN_ERROR_INVAL;
                    }
                    break;
                }
                token = &parser->tokens[token->parent_index];
            }
#else
            for (i = parser->token_count - 1; i >= 0; i--) {
                token = &parser->tokens[i];
                if (token->start != NULL && token->strlen == -1) {
                    if (token->type != type) {
                        return JSMN_ERROR_INVAL;
                    }
                    parser->parent_index = -1;
                    token->strlen = (parser->pos + 1) - (token->start - js);
                    break;
                }
            }
            /* Error if unmatched closing bracket */
            if (i == -1) {
                return JSMN_ERROR_INVAL;
            }
            for (; i >= 0; i--) {
                token = &parser->tokens[i];
                if (token->start != NULL && token->strlen == -1) {
                    parser->parent_index = i;
                    break;
                }
            }
#endif
            break;
        case '\"':
            r = jsmn_parse_string(parser, js, len);
            if (r < 0) {
                return r;
            }
            count++;
            if (parser->parent_index != -1 && parser->tokens != NULL) {
                parser->tokens[parser->parent_index].child_count++;
            }
            break;
        case '\t':
        case '\r':
        case '\n':
        case ' ':
            break;
        case ':':
            parser->parent_index = parser->token_count - 1;
            break;
        case ',':
            if (parser->tokens != NULL && parser->parent_index != -1 &&
                parser->tokens[parser->parent_index].type != JSMN_ARRAY &&
                parser->tokens[parser->parent_index].type != JSMN_OBJECT) {
#ifdef JSMN_PARENT_LINKS
                parser->parent_index =
                    parser->tokens[parser->parent_index].parent_index;
#else
                for (i = parser->token_count - 1; i >= 0; i--) {
                    if (parser->tokens[i].type == JSMN_ARRAY ||
                        parser->tokens[i].type == JSMN_OBJECT) {
                        if (parser->tokens[i].start != NULL &&
                            parser->tokens[i].strlen == -1) {
                            parser->parent_index = i;
                            break;
                        }
                    }
                }
#endif
            }
            break;
#ifdef JSMN_STRICT
        /* In strict mode primitives are: numbers and booleans */
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 't':
        case 'f':
        case 'n':
            /* And they must not be keys of the object */
            if (parser->tokens != NULL && parser->parent_index != -1) {
                const jsmn_token_t *t = &parser->tokens[parser->parent_index];
                if (t->type == JSMN_OBJECT ||
                    (t->type == JSMN_STRING && t->child_count != 0)) {
                    return JSMN_ERROR_INVAL;
                }
            }
#else
        /* In non-strict mode every unquoted value is a primitive */
        default:
#endif
            r = jsmn_parse_primitive(parser, js, len);
            if (r < 0) {
                return r;
            }
            count++;
            if (parser->parent_index != -1 && parser->tokens != NULL) {
                parser->tokens[parser->parent_index].child_count++;
            }
            break;

#ifdef JSMN_STRICT
        /* Unexpected char in strict mode */
        default:
            return JSMN_ERROR_INVAL;
#endif
        }
    }

    if (parser->tokens != NULL) {
        for (i = parser->token_count - 1; i >= 0; i--) {
            /* Unmatched opened object or array */
            if (parser->tokens[i].start != NULL &&
                parser->tokens[i].strlen == -1) {
                return JSMN_ERROR_PART;
            }
        }
    }

    return count;
}

jsmn_token_t *jsmn_token_ref(jsmn_parser_t *parser, int index) {
    if ((index < 0) || (index >= parser->token_count)) {
        return NULL;
    } else {
        return &parser->tokens[index];
    }
}

jsmn_token_type_t jsmn_token_type(jsmn_token_t *token) {
    if (token == NULL) {
        return JSMN_UNDEFINED;
    } else {
        return token->type;
    }
}

const char *jsmn_token_string(jsmn_token_t *token) {
    if (token == NULL) {
        return NULL;
    } else {
        return token->start;
    }
}

int jsmn_token_strlen(jsmn_token_t *token) {
    if (token == NULL) {
        return 0;
    } else {
        return token->strlen;
    }
}

int jsmn_token_level(jsmn_token_t *token) {
    if (token == NULL) {
        return -1;
    } else {
        return token->level;
    }
}

int jsmn_parent_of(jsmn_parser_t *parser, int token_index) {
    int level = jsmn_token_level(jsmn_token_ref(parser, token_index));
    if (level <= 0) {
        // if level is 0, we're already at top level so there's no parent.
        // if level is -1, then token_index was invalid.
        return -1;
    } else {
        // search backwards from token_index - 1 for the first token with a
        // lower level number.
        for (int i = token_index - 1; i >= 0; --i) {
            if (jsmn_token_level(jsmn_token_ref(parser, i)) < level) {
                return i;
            }
        }
        // not found: no parent
        return -1;
    }
}

int jsmn_sibling_of(jsmn_parser_t *parser, int token_index) {
    int level = jsmn_token_level(jsmn_token_ref(parser, token_index));
    if (level <= 0) {
        // if level is 0, we're already at top level so there's no sibling.
        // if level is -1, then token_index was invalid.
        return -1;
    }
    // search forward for the next token with the same level number.
    for (int i = token_index + 1; i < parser->token_count; i++) {
        if (jsmn_token_level(jsmn_token_ref(parser, i)) == level) {
            // found the sibling
            return i;
        }
    }
    // ran out of tokens without finding a sibling
    return -1;
}

int jsmn_child_of(jsmn_parser_t *parser, int token_index) {
    int level = jsmn_token_level(jsmn_token_ref(parser, token_index));
    if (level < 0) {
        // if level is -1, then token_index was invalid.
        return -1;
    }
    if (jsmn_token_level(jsmn_token_ref(parser, token_index + 1)) ==
        level + 1) {
        // next token is one level deeper: it's a child...
        return token_index + 1;
    }
    // next token doesn't exist or has different level: not a child
    return -1;
}

bool jsmn_token_stringeq(jsmn_token_t *token, const char *literal) {
    // printf("stringeq tok '%.*s'\n", jsmn_token_strlen(token),
    // jsmn_token_string(token));
    return strncmp(literal, jsmn_token_string(token),
                   jsmn_token_strlen(token)) == 0;
}

int jsmn_token_find(jsmn_parser_t *parser, const char *literal) {
    for (int i = 0; i < parser->token_count; i++) {
        if (jsmn_token_stringeq(jsmn_token_ref(parser, i), literal)) {
            // got a match
            return i;
        }
    }
    // ran out of tokens without finding a sibling
    return -1;
}

bool jsmn_token_is_array(jsmn_token_t *token) {
    if (token == NULL) {
        return false;
    } else {
        return token->type & JSMN_ARRAY;
    }
}

bool jsmn_token_is_boolean(jsmn_token_t *token) {
    return jsmn_token_is_false(token) || jsmn_token_is_true(token);
}

bool jsmn_token_is_false(jsmn_token_t *token) {
    return jsmn_token_is_primitive(token) && (*jsmn_token_string(token) == 'f');
}

bool jsmn_token_is_float(jsmn_token_t *token) {
    return jsmn_token_is_number(token) &&
           memchr(jsmn_token_string(token), '.', jsmn_token_strlen(token));
}

bool jsmn_token_is_integer(jsmn_token_t *token) {
    return jsmn_token_is_number(token) && !jsmn_token_is_float(token);
}

bool jsmn_token_is_null(jsmn_token_t *token) {
    return jsmn_token_is_primitive(token) && (*jsmn_token_string(token) == 'n');
}

bool jsmn_token_is_number(jsmn_token_t *token) {
    if (!jsmn_token_is_primitive(token)) {
        return false;
    } else {
        unsigned char c = *jsmn_token_string(token);
        return ((c >= '0') && (c <= '9')) || (c == '-');
    }
}

bool jsmn_token_is_object(jsmn_token_t *token) {
    if (token == NULL) {
        return false;
    } else {
        return token->type & JSMN_OBJECT;
    }
}

bool jsmn_token_is_primitive(jsmn_token_t *token) {
    if (token == NULL) {
        return false;
    } else {
        return token->type & JSMN_PRIMITIVE;
    }
}

bool jsmn_token_is_string(jsmn_token_t *token) {
    if (token == NULL) {
        return false;
    } else {
        return token->type & JSMN_STRING;
    }
}

bool jsmn_token_is_true(jsmn_token_t *token) {
    return jsmn_token_is_primitive(token) && (*jsmn_token_string(token) == 't');
}

// *****************************************************************************
// local (private) functions

static void reset_parser(jsmn_parser_t *parser) {
    memset(parser->tokens, 0, sizeof(jsmn_token_t) * parser->num_tokens);
    parser->pos = 0;
    parser->token_count = 0;
    parser->parent_index = -1;
    parser->level = 0;
}

static jsmn_token_t *jsmn_alloc_token(jsmn_parser_t *parser) {
    jsmn_token_t *tok;
    if (parser->token_count >= parser->num_tokens) {
        return NULL;
    }
    tok = &parser->tokens[parser->token_count++];
    tok->start = NULL;
    tok->strlen = -1;
    tok->child_count = 0;
#ifdef JSMN_PARENT_LINKS
    tok->parent_index = -1;
#endif
    tok->level = parser->level;
    return tok;
}

static void jsmn_fill_token(jsmn_token_t *token, const jsmn_token_type_t type,
                            const char *start, int length) {
    token->type = type;
    token->start = start;
    token->strlen = length;
    token->child_count = 0;
}

static int jsmn_parse_primitive(jsmn_parser_t *parser, const char *js,
                                const size_t len) {
    jsmn_token_t *token;
    int start; // index, not char pointer!

    start = parser->pos;

    for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
        switch (js[parser->pos]) {
#ifndef JSMN_STRICT
        /* In strict mode primitive must be followed by "," or "}" or "]" */
        case ':':
#endif
        case '\t':
        case '\r':
        case '\n':
        case ' ':
        case ',':
        case ']':
        case '}':
            goto found;
        default:
            /* to quiet a warning from gcc*/
            break;
        }
        if (js[parser->pos] < 32 || js[parser->pos] >= 127) {
            parser->pos = start;
            return JSMN_ERROR_INVAL;
        }
    }
#ifdef JSMN_STRICT
    /* In strict mode primitive must be followed by a comma/object/array */
    parser->pos = start;
    return JSMN_ERROR_PART;
#endif

found:
    if (parser->tokens == NULL) {
        parser->pos--;
        return 0;
    }
    token = jsmn_alloc_token(parser);
    if (token == NULL) {
        parser->pos = start;
        return JSMN_ERROR_NOMEM;
    }
    jsmn_fill_token(token, JSMN_PRIMITIVE, &js[start], parser->pos - start);
#ifdef JSMN_PARENT_LINKS
    token->parent_index = parser->parent_index;
#endif
    parser->pos--;
    return 0;
}

static int jsmn_parse_string(jsmn_parser_t *parser, const char *js,
                             const size_t len) {
    jsmn_token_t *token;

    int start = parser->pos; // index, not char pointer!

    /* Skip starting quote */
    parser->pos++;

    for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
        char c = js[parser->pos];

        /* Quote: end of string */
        if (c == '\"') {
            if (parser->tokens == NULL) {
                return 0;
            }
            token = jsmn_alloc_token(parser);
            if (token == NULL) {
                parser->pos = start;
                return JSMN_ERROR_NOMEM;
            }
            jsmn_fill_token(token, JSMN_STRING, &js[start + 1],
                            parser->pos - start - 1);
#ifdef JSMN_PARENT_LINKS
            token->parent_index = parser->parent_index;
#endif
            return 0;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\' && parser->pos + 1 < len) {
            int i;
            parser->pos++;
            switch (js[parser->pos]) {
            /* Allowed escaped symbols */
            case '\"':
            case '/':
            case '\\':
            case 'b':
            case 'f':
            case 'r':
            case 'n':
            case 't':
                break;
            /* Allows escaped symbol \uXXXX */
            case 'u':
                parser->pos++;
                for (i = 0;
                     i < 4 && parser->pos < len && js[parser->pos] != '\0';
                     i++) {
                    /* If it isn't a hex character we have an error */
                    if (!((js[parser->pos] >= 48 &&
                           js[parser->pos] <= 57) || /* 0-9 */
                          (js[parser->pos] >= 65 &&
                           js[parser->pos] <= 70) || /* A-F */
                          (js[parser->pos] >= 97 &&
                           js[parser->pos] <= 102))) { /* a-f */
                        parser->pos = start;
                        return JSMN_ERROR_INVAL;
                    }
                    parser->pos++;
                }
                parser->pos--;
                break;
            /* Unexpected symbol */
            default:
                parser->pos = start;
                return JSMN_ERROR_INVAL;
            }
        }
    }
    parser->pos = start;
    return JSMN_ERROR_PART;
}
