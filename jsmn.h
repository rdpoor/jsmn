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
#ifndef JSMN_H
#define JSMN_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
  JSMN_UNDEFINED = 0,
  JSMN_OBJECT = 1 << 0,
  JSMN_ARRAY = 1 << 1,
  JSMN_STRING = 1 << 2,
  JSMN_PRIMITIVE = 1 << 3,
} jsmn_token_type_t;

typedef enum {
  /* Not enough tokens were provided */
  JSMN_ERROR_NOMEM = -1,
  /* Invalid character inside JSON string */
  JSMN_ERROR_INVAL = -2,
  /* The string is not a full JSON packet, more bytes expected */
  JSMN_ERROR_PART = -3
} jsmn_err_t;

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	pointer to the first char of the token string
 * strlen number of characters in the token string.
 */
typedef struct {
  jsmn_token_type_t type;
  const char *start;     // start of token string
  int strlen;            // length of token string
  int child_count;       // number of nested tokens within OBJECT or ARRAy
#ifdef JSMN_PARENT_LINKS
  int parent_index;      // index to token that contains this token
#endif
  int level;
} jsmn_token_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string.
 */
typedef struct {
  jsmn_token_t *tokens;     // array of tokens
  unsigned int num_tokens;  // number of available tokens
  unsigned int token_count; // number of allocated tokens
  unsigned int pos;         // offset in the JSON string
  int parent_index;         // index of containing node (array or object) or -1
  int level;
} jsmn_parser_t;

/**
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser_t *parser, jsmn_token_t *tokens, unsigned int num_tokens);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each
 * describing a single JSON object.
 */
int jsmn_parse(jsmn_parser_t *parser, const char *js, const size_t len);

/**
 * @brief Return a token, referenced by index.  Return NULL if out of range.
 */
jsmn_token_t *jsmn_token_ref(jsmn_parser_t *parser, int index);

/**
 * @brief Return the primitive type of the token: JSMN_OBJECT, JSMN_ARRAY,
 * JSMN_STRING, JSMN_PRIMITIVE.  See jsmn_tokan_is_xxx() for finer-grained
 * typing.
 */
jsmn_token_type_t jsmn_token_type(jsmn_token_t *token);

/**
 * @brief Return a pointer to the first character of the token's underlying
 * string.
 */
const char *jsmn_token_string(jsmn_token_t *token);

/**
 * @brief Return the number of bytes in the token's underlying string.
 */
int jsmn_token_strlen(jsmn_token_t *token);

/**
 * @brief Return the hierarchical level of the given token: 0 for top level,
 * 1 for nested 1 deep, etc.  Return -1 on NULL token.
 */
int jsmn_token_level(jsmn_token_t *token);

/**
 * @brief Return the index of the parent of this token, if any, else -1.
 */
int jsmn_parent_of(jsmn_parser_t *parser, int token_index);

/**
 * @brief Return the index of the next sibling of this token, if any, else -1.
 */
int jsmn_sibling_of(jsmn_parser_t *parser, int token_index);

/**
 * @brief Return the index of the first child of this token, if any.
 */
int jsmn_child_of(jsmn_parser_t *parser, int token_index);

bool jsmn_token_is_array(jsmn_token_t *token);
bool jsmn_token_is_boolean(jsmn_token_t *token);
bool jsmn_token_is_false(jsmn_token_t *token);
bool jsmn_token_is_float(jsmn_token_t *token);
bool jsmn_token_is_integer(jsmn_token_t *token);
bool jsmn_token_is_null(jsmn_token_t *token);
bool jsmn_token_is_number(jsmn_token_t *token);
bool jsmn_token_is_object(jsmn_token_t *token);
bool jsmn_token_is_primitive(jsmn_token_t *token);
bool jsmn_token_is_string(jsmn_token_t *token);
bool jsmn_token_is_true(jsmn_token_t *token);
bool jsmn_token_is_array(jsmn_token_t *token);

#ifdef __cplusplus
}
#endif

#endif /* JSMN_H */
