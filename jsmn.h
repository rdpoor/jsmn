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
  JSMN_PRIMITIVE = 1 << 3
} jsmn_token_type_t;

enum jsmnerr {
  /* Not enough tokens were provided */
  JSMN_ERROR_NOMEM = -1,
  /* Invalid character inside JSON string */
  JSMN_ERROR_INVAL = -2,
  /* The string is not a full JSON packet, more bytes expected */
  JSMN_ERROR_PART = -3
};

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
typedef struct {
  jsmn_token_type_t type;
  // int start;
  // int end;
  const char *start;
  int length;
  int size;
#ifdef JSMN_PARENT_LINKS
  int parent;
#endif
} jsmn_token_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string.
 */
typedef struct {
  jsmn_token_t *tokens;     // array of tokens
  unsigned int num_tokens;  // number of tokens
  unsigned int toknext;     // index of next available token to allocate
  unsigned int pos;         // offset in the JSON string
  int toksuper;         /* superior token node, e.g. parent object or array */
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

jsmn_token_type_t jsmn_token_type(jsmn_token_t *token);
const char *jsmn_token_string(jsmn_token_t *token);
int jsmn_token_length(jsmn_token_t *token);

#ifdef __cplusplus
}
#endif

#endif /* JSMN_H */
