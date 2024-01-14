#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "testutil.h"

int test_empty(void) {
  check(parse("{}", 1, 1, JSMN_OBJECT, 0, 2, 0));
  check(parse("[]", 1, 1, JSMN_ARRAY, 0, 2, 0));
  check(parse("[{},{}]", 3, 3, JSMN_ARRAY, 0, 7, 2, JSMN_OBJECT, 1, 3, 0,
              JSMN_OBJECT, 4, 6, 0));
  return 0;
}

int test_object(void) {
  check(parse("{\"a\":0}", 3, 3, JSMN_OBJECT, 0, 7, 1, JSMN_STRING, "a", 1,
              JSMN_PRIMITIVE, "0"));
  check(parse("{\"a\":[]}", 3, 3, JSMN_OBJECT, 0, 8, 1, JSMN_STRING, "a", 1,
              JSMN_ARRAY, 5, 7, 0));
  check(parse("{\"a\":{},\"b\":{}}", 5, 5, JSMN_OBJECT, -1, -1, 2, JSMN_STRING,
              "a", 1, JSMN_OBJECT, -1, -1, 0, JSMN_STRING, "b", 1, JSMN_OBJECT,
              -1, -1, 0));
  check(parse("{\n \"Day\": 26,\n \"Month\": 9,\n \"Year\": 12\n }", 7, 7,
              JSMN_OBJECT, -1, -1, 3, JSMN_STRING, "Day", 1, JSMN_PRIMITIVE,
              "26", JSMN_STRING, "Month", 1, JSMN_PRIMITIVE, "9", JSMN_STRING,
              "Year", 1, JSMN_PRIMITIVE, "12"));
  check(parse("{\"a\": 0, \"b\": \"c\"}", 5, 5, JSMN_OBJECT, -1, -1, 2,
              JSMN_STRING, "a", 1, JSMN_PRIMITIVE, "0", JSMN_STRING, "b", 1,
              JSMN_STRING, "c", 0));

#ifdef JSMN_STRICT
  check(parse("{\"a\"\n0}", JSMN_ERROR_INVAL, 3));
  check(parse("{\"a\", 0}", JSMN_ERROR_INVAL, 3));
  check(parse("{\"a\": {2}}", JSMN_ERROR_INVAL, 3));
  check(parse("{\"a\": {2: 3}}", JSMN_ERROR_INVAL, 3));
  check(parse("{\"a\": {\"a\": 2 3}}", JSMN_ERROR_INVAL, 5));
/* FIXME */
/*check(parse("{\"a\"}", JSMN_ERROR_INVAL, 2));*/
/*check(parse("{\"a\": 1, \"b\"}", JSMN_ERROR_INVAL, 4));*/
/*check(parse("{\"a\",\"b\":1}", JSMN_ERROR_INVAL, 4));*/
/*check(parse("{\"a\":1,}", JSMN_ERROR_INVAL, 4));*/
/*check(parse("{\"a\":\"b\":\"c\"}", JSMN_ERROR_INVAL, 4));*/
/*check(parse("{,}", JSMN_ERROR_INVAL, 4));*/
#endif
  return 0;
}

int test_array(void) {
  /* FIXME */
  /*check(parse("[10}", JSMN_ERROR_INVAL, 3));*/
  /*check(parse("[1,,3]", JSMN_ERROR_INVAL, 3)*/
  check(parse("[10]", 2, 2, JSMN_ARRAY, -1, -1, 1, JSMN_PRIMITIVE, "10"));
  check(parse("{\"a\": 1]", JSMN_ERROR_INVAL, 3));
  /* FIXME */
  /*check(parse("[\"a\": 1]", JSMN_ERROR_INVAL, 3));*/
  return 0;
}

int test_primitive(void) {
  check(parse("{\"boolVar\" : true }", 3, 3, JSMN_OBJECT, -1, -1, 1,
              JSMN_STRING, "boolVar", 1, JSMN_PRIMITIVE, "true"));
  check(parse("{\"boolVar\" : false }", 3, 3, JSMN_OBJECT, -1, -1, 1,
              JSMN_STRING, "boolVar", 1, JSMN_PRIMITIVE, "false"));
  check(parse("{\"nullVar\" : null }", 3, 3, JSMN_OBJECT, -1, -1, 1,
              JSMN_STRING, "nullVar", 1, JSMN_PRIMITIVE, "null"));
  check(parse("{\"intVar\" : 12}", 3, 3, JSMN_OBJECT, -1, -1, 1, JSMN_STRING,
              "intVar", 1, JSMN_PRIMITIVE, "12"));
  check(parse("{\"floatVar\" : 12.345}", 3, 3, JSMN_OBJECT, -1, -1, 1,
              JSMN_STRING, "floatVar", 1, JSMN_PRIMITIVE, "12.345"));
  return 0;
}

int test_string(void) {
  check(parse("{\"strVar\" : \"hello world\"}", 3, 3, JSMN_OBJECT, -1, -1, 1,
              JSMN_STRING, "strVar", 1, JSMN_STRING, "hello world", 0));
  check(parse("{\"strVar\" : \"escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\\"}", 3, 3,
              JSMN_OBJECT, -1, -1, 1, JSMN_STRING, "strVar", 1, JSMN_STRING,
              "escapes: \\/\\r\\n\\t\\b\\f\\\"\\\\", 0));
  check(parse("{\"strVar\": \"\"}", 3, 3, JSMN_OBJECT, -1, -1, 1, JSMN_STRING,
              "strVar", 1, JSMN_STRING, "", 0));
  check(parse("{\"a\":\"\\uAbcD\"}", 3, 3, JSMN_OBJECT, -1, -1, 1, JSMN_STRING,
              "a", 1, JSMN_STRING, "\\uAbcD", 0));
  check(parse("{\"a\":\"str\\u0000\"}", 3, 3, JSMN_OBJECT, -1, -1, 1,
              JSMN_STRING, "a", 1, JSMN_STRING, "str\\u0000", 0));
  check(parse("{\"a\":\"\\uFFFFstr\"}", 3, 3, JSMN_OBJECT, -1, -1, 1,
              JSMN_STRING, "a", 1, JSMN_STRING, "\\uFFFFstr", 0));
  check(parse("{\"a\":[\"\\u0280\"]}", 4, 4, JSMN_OBJECT, -1, -1, 1,
              JSMN_STRING, "a", 1, JSMN_ARRAY, -1, -1, 1, JSMN_STRING,
              "\\u0280", 0));

  check(parse("{\"a\":\"str\\uFFGFstr\"}", JSMN_ERROR_INVAL, 3));
  check(parse("{\"a\":\"str\\u@FfF\"}", JSMN_ERROR_INVAL, 3));
  check(parse("{{\"a\":[\"\\u028\"]}", JSMN_ERROR_INVAL, 4));
  return 0;
}

int test_partial_string(void) {
  int r;
  unsigned long i;
  jsmn_parser_t p;
  jsmn_token_t tok[5];
  const char *js = "{\"x\": \"va\\\\ue\", \"y\": \"value y\"}";

  jsmn_init(&p, tok, sizeof(tok) / sizeof(tok[0]));
  for (i = 1; i <= strlen(js); i++) {
    r = jsmn_parse(&p, js, i);
    if (i == strlen(js)) {
      check(r == 5);
      check(tokeq(js, tok, 5, JSMN_OBJECT, -1, -1, 2, JSMN_STRING, "x", 1,
                  JSMN_STRING, "va\\\\ue", 0, JSMN_STRING, "y", 1, JSMN_STRING,
                  "value y", 0));
    } else {
      check(r == JSMN_ERROR_PART);
    }
  }
  return 0;
}

int test_partial_array(void) {
#ifdef JSMN_STRICT
  int r;
  unsigned long i;
  jsmn_parser_t p;
  jsmn_token_t tok[10];
  const char *js = "[ 1, true, [123, \"hello\"]]";

  jsmn_init(&p, tok, sizeof(tok) / sizeof(tok[0]));
  for (i = 1; i <= strlen(js); i++) {
    r = jsmn_parse(&p, js, i);
    if (i == strlen(js)) {
      check(r == 6);
      check(tokeq(js, tok, 6, JSMN_ARRAY, -1, -1, 3, JSMN_PRIMITIVE, "1",
                  JSMN_PRIMITIVE, "true", JSMN_ARRAY, -1, -1, 2, JSMN_PRIMITIVE,
                  "123", JSMN_STRING, "hello", 0));
    } else {
      check(r == JSMN_ERROR_PART);
    }
  }
#endif
  return 0;
}

int test_array_nomem(void) {
  int r;
  jsmn_parser_t p;
  jsmn_token_t tokens[10];
  const char *js;

  js = "  [ 1, true, [123, \"hello\"]]";

  for (int i = 0; i < 6; i++) {
    jsmn_init(&p, tokens, i);
    r = jsmn_parse(&p, js, strlen(js));
    check(r == JSMN_ERROR_NOMEM);
  }

  jsmn_init(&p, tokens, sizeof(tokens)/sizeof(tokens[0]));
  r = jsmn_parse(&p, js, strlen(js));
  check(r >= 0);
  check(tokeq(js, tokens, 4, JSMN_ARRAY, -1, -1, 3, JSMN_PRIMITIVE, "1",
              JSMN_PRIMITIVE, "true", JSMN_ARRAY, -1, -1, 2, JSMN_PRIMITIVE,
              "123", JSMN_STRING, "hello", 0));
  return 0;
}

int test_unquoted_keys(void) {
#ifndef JSMN_STRICT
  int r;
  jsmn_parser_t p;
  jsmn_token_t tok[10];
  const char *js;

  jsmn_init(&p, tok, 10);
  js = "key1: \"value\"\nkey2 : 123";

  r = jsmn_parse(&p, js, strlen(js));
  check(r >= 0);
  check(tokeq(js, tok, 4, JSMN_PRIMITIVE, "key1", JSMN_STRING, "value", 0,
              JSMN_PRIMITIVE, "key2", JSMN_PRIMITIVE, "123"));
#endif
  return 0;
}

int test_issue_22(void) {
  int r;
  jsmn_parser_t p;
  jsmn_token_t tokens[128];
  const char *js;

  js =
      "{ \"height\":10, \"layers\":[ { \"data\":[6,6], \"height\":10, "
      "\"name\":\"Calque de Tile 1\", \"opacity\":1, \"type\":\"tilelayer\", "
      "\"visible\":true, \"width\":10, \"x\":0, \"y\":0 }], "
      "\"orientation\":\"orthogonal\", \"properties\": { }, \"tileheight\":32, "
      "\"tilesets\":[ { \"firstgid\":1, \"image\":\"..\\/images\\/tiles.png\", "
      "\"imageheight\":64, \"imagewidth\":160, \"margin\":0, "
      "\"name\":\"Tiles\", "
      "\"properties\":{}, \"spacing\":0, \"tileheight\":32, \"tilewidth\":32 "
      "}], "
      "\"tilewidth\":32, \"version\":1, \"width\":10 }";
  jsmn_init(&p, tokens, 128);
  r = jsmn_parse(&p, js, strlen(js));
  check(r >= 0);
  return 0;
}

int test_issue_27(void) {
  const char *js =
      "{ \"name\" : \"Jack\", \"age\" : 27 } { \"name\" : \"Anna\", ";
  check(parse(js, JSMN_ERROR_PART, 8));
  return 0;
}

int test_input_length(void) {
  const char *js;
  int r;
  jsmn_parser_t p;
  jsmn_token_t tokens[10];

  js = "{\"a\": 0}garbage";

  jsmn_init(&p, tokens, 10);
  r = jsmn_parse(&p, js, 8);
  check(r == 3);
  check(tokeq(js, tokens, 3, JSMN_OBJECT, -1, -1, 1, JSMN_STRING, "a", 1,
              JSMN_PRIMITIVE, "0"));
  return 0;
}

int test_count(void) {
  jsmn_parser_t p;
  const char *js;

  js = "{}";
  jsmn_init(&p, NULL, 0);
  check(jsmn_parse(&p, js, strlen(js)) == 1);

  js = "[]";
  jsmn_init(&p, NULL, 0);
  check(jsmn_parse(&p, js, strlen(js)) == 1);

  js = "[[]]";
  jsmn_init(&p, NULL, 0);
  check(jsmn_parse(&p, js, strlen(js)) == 2);

  js = "[[], []]";
  jsmn_init(&p, NULL, 0);
  check(jsmn_parse(&p, js, strlen(js)) == 3);

  js = "[[], []]";
  jsmn_init(&p, NULL, 0);
  check(jsmn_parse(&p, js, strlen(js)) == 3);

  js = "[[], [[]], [[], []]]";
  jsmn_init(&p, NULL, 0);
  check(jsmn_parse(&p, js, strlen(js)) == 7);

  js = "[\"a\", [[], []]]";
  jsmn_init(&p, NULL, 0);
  check(jsmn_parse(&p, js, strlen(js)) == 5);

  js = "[[], \"[], [[]]\", [[]]]";
  jsmn_init(&p, NULL, 0);
  check(jsmn_parse(&p, js, strlen(js)) == 5);

  js = "[1, 2, 3]";
  jsmn_init(&p, NULL, 0);
  check(jsmn_parse(&p, js, strlen(js)) == 4);

  js = "[1, 2, [3, \"a\"], null]";
  jsmn_init(&p, NULL, 0);
  check(jsmn_parse(&p, js, strlen(js)) == 7);

  return 0;
}

int test_nonstrict(void) {
#ifndef JSMN_STRICT
  const char *js;
  js = "a: 0garbage";
  check(parse(js, 2, 2, JSMN_PRIMITIVE, "a", JSMN_PRIMITIVE, "0garbage"));

  js = "Day : 26\nMonth : Sep\n\nYear: 12";
  check(parse(js, 6, 6, JSMN_PRIMITIVE, "Day", JSMN_PRIMITIVE, "26",
              JSMN_PRIMITIVE, "Month", JSMN_PRIMITIVE, "Sep", JSMN_PRIMITIVE,
              "Year", JSMN_PRIMITIVE, "12"));

  /* nested {s don't cause a parse error. */
  js = "\"key {1\": 1234";
  check(parse(js, 2, 2, JSMN_STRING, "key {1", 1, JSMN_PRIMITIVE, "1234"));

#endif
  return 0;
}

int test_unmatched_brackets(void) {
  const char *js;
  js = "\"key 1\": 1234}";
  check(parse(js, JSMN_ERROR_INVAL, 2));
  js = "{\"key 1\": 1234";
  check(parse(js, JSMN_ERROR_PART, 3));
  js = "{\"key 1\": 1234}}";
  check(parse(js, JSMN_ERROR_INVAL, 3));
  js = "\"key 1\"}: 1234";
  check(parse(js, JSMN_ERROR_INVAL, 3));
  js = "{\"key {1\": 1234}";
  check(parse(js, 3, 3, JSMN_OBJECT, 0, 16, 1, JSMN_STRING, "key {1", 1,
              JSMN_PRIMITIVE, "1234"));
  js = "{\"key 1\":{\"key 2\": 1234}";
  check(parse(js, JSMN_ERROR_PART, 5));
  return 0;
}

int test_object_key(void) {
  const char *js;

  js = "{\"key\": 1}";
  check(parse(js, 3, 3, JSMN_OBJECT, 0, 10, 1, JSMN_STRING, "key", 1,
              JSMN_PRIMITIVE, "1"));
#ifdef JSMN_STRICT
  js = "{true: 1}";
  check(parse(js, JSMN_ERROR_INVAL, 3));
  js = "{1: 1}";
  check(parse(js, JSMN_ERROR_INVAL, 3));
  js = "{{\"key\": 1}: 2}";
  check(parse(js, JSMN_ERROR_INVAL, 5));
  js = "{[1,2]: 2}";
  check(parse(js, JSMN_ERROR_INVAL, 5));
#endif
  return 0;
}

int test_token_types(void) {
    jsmn_token_t tokens[10];
    jsmn_parser_t parser;
    const char *js = "[1, -1.2, true, false, null, \"a\", {\"x\":\"y\"}]";
    jsmn_token_t *token;

    jsmn_init(&parser, tokens, sizeof(tokens) / sizeof(tokens[0]));
    check(jsmn_parse(&parser, js, strlen(js)) == 10);

    // [
    token = jsmn_token_ref(&parser, 0);
    check(jsmn_token_is_array(token));
    check(!jsmn_token_is_boolean(token));
    check(!jsmn_token_is_false(token));
    check(!jsmn_token_is_float(token));
    check(!jsmn_token_is_integer(token));
    check(!jsmn_token_is_null(token));
    check(!jsmn_token_is_number(token));
    check(!jsmn_token_is_object(token));
    check(!jsmn_token_is_primitive(token));
    check(!jsmn_token_is_string(token));
    check(!jsmn_token_is_true(token));

    // 1
    token = jsmn_token_ref(&parser, 1);
    check(!jsmn_token_is_array(token));
    check(!jsmn_token_is_boolean(token));
    check(!jsmn_token_is_false(token));
    check(!jsmn_token_is_float(token));
    check(jsmn_token_is_integer(token));
    check(!jsmn_token_is_null(token));
    check(jsmn_token_is_number(token));
    check(!jsmn_token_is_object(token));
    check(jsmn_token_is_primitive(token));
    check(!jsmn_token_is_string(token));
    check(!jsmn_token_is_true(token));

    // -1.2
    token = jsmn_token_ref(&parser, 2);
    check(!jsmn_token_is_array(token));
    check(!jsmn_token_is_boolean(token));
    check(!jsmn_token_is_false(token));
    check(jsmn_token_is_float(token));
    check(!jsmn_token_is_integer(token));
    check(!jsmn_token_is_null(token));
    check(jsmn_token_is_number(token));
    check(!jsmn_token_is_object(token));
    check(jsmn_token_is_primitive(token));
    check(!jsmn_token_is_string(token));
    check(!jsmn_token_is_true(token));

    // true
    token = jsmn_token_ref(&parser, 3);
    check(!jsmn_token_is_array(token));
    check(jsmn_token_is_boolean(token));
    check(!jsmn_token_is_false(token));
    check(!jsmn_token_is_float(token));
    check(!jsmn_token_is_integer(token));
    check(!jsmn_token_is_null(token));
    check(!jsmn_token_is_number(token));
    check(!jsmn_token_is_object(token));
    check(jsmn_token_is_primitive(token));
    check(!jsmn_token_is_string(token));
    check(jsmn_token_is_true(token));

    // false
    token = jsmn_token_ref(&parser, 4);
    check(!jsmn_token_is_array(token));
    check(jsmn_token_is_boolean(token));
    check(jsmn_token_is_false(token));
    check(!jsmn_token_is_float(token));
    check(!jsmn_token_is_integer(token));
    check(!jsmn_token_is_null(token));
    check(!jsmn_token_is_number(token));
    check(!jsmn_token_is_object(token));
    check(jsmn_token_is_primitive(token));
    check(!jsmn_token_is_string(token));
    check(!jsmn_token_is_true(token));

    // null
    token = jsmn_token_ref(&parser, 5);
    check(!jsmn_token_is_array(token));
    check(!jsmn_token_is_boolean(token));
    check(!jsmn_token_is_false(token));
    check(!jsmn_token_is_float(token));
    check(!jsmn_token_is_integer(token));
    check(jsmn_token_is_null(token));
    check(!jsmn_token_is_number(token));
    check(!jsmn_token_is_object(token));
    check(jsmn_token_is_primitive(token));
    check(!jsmn_token_is_string(token));
    check(!jsmn_token_is_true(token));

    // "a"
    token = jsmn_token_ref(&parser, 6);
    check(!jsmn_token_is_array(token));
    check(!jsmn_token_is_boolean(token));
    check(!jsmn_token_is_false(token));
    check(!jsmn_token_is_float(token));
    check(!jsmn_token_is_integer(token));
    check(!jsmn_token_is_null(token));
    check(!jsmn_token_is_number(token));
    check(!jsmn_token_is_object(token));
    check(!jsmn_token_is_primitive(token));
    check(jsmn_token_is_string(token));
    check(!jsmn_token_is_true(token));

    // {
    token = jsmn_token_ref(&parser, 7);
    check(!jsmn_token_is_array(token));
    check(!jsmn_token_is_boolean(token));
    check(!jsmn_token_is_false(token));
    check(!jsmn_token_is_float(token));
    check(!jsmn_token_is_integer(token));
    check(!jsmn_token_is_null(token));
    check(!jsmn_token_is_number(token));
    check(jsmn_token_is_object(token));
    check(!jsmn_token_is_primitive(token));
    check(!jsmn_token_is_string(token));
    check(!jsmn_token_is_true(token));

    // pathology
    token = NULL;
    check(!jsmn_token_is_array(token));
    check(!jsmn_token_is_boolean(token));
    check(!jsmn_token_is_false(token));
    check(!jsmn_token_is_float(token));
    check(!jsmn_token_is_integer(token));
    check(!jsmn_token_is_null(token));
    check(!jsmn_token_is_number(token));
    check(!jsmn_token_is_object(token));
    check(!jsmn_token_is_primitive(token));
    check(!jsmn_token_is_string(token));
    check(!jsmn_token_is_true(token));

    // more pathology
    const char *js2 = "[\"-3.14159\"]";
    check(jsmn_parse(&parser, js2, strlen(js2)) == 2);

    // "-3.14159" (note: that's a string, not a number)
    token = jsmn_token_ref(&parser, 1);
    check(!jsmn_token_is_array(token));
    check(!jsmn_token_is_boolean(token));
    check(!jsmn_token_is_false(token));
    check(!jsmn_token_is_float(token));
    check(!jsmn_token_is_integer(token));
    check(!jsmn_token_is_null(token));
    check(!jsmn_token_is_number(token));
    check(!jsmn_token_is_object(token));
    check(!jsmn_token_is_primitive(token));
    check(jsmn_token_is_string(token));
    check(!jsmn_token_is_true(token));

    return 0;
}


int test_hierarchy(void) {
    jsmn_token_t tokens[10];
    jsmn_parser_t parser;
    int tok_index;
    jsmn_token_t *tok;

    jsmn_init(&parser, tokens, sizeof(tokens) / sizeof(tokens[0]));

    //          index: 0  1   2    3   4  5   6     7   8
    const char *js1 = "{\"a\":1, \"b\":{\"c\":3}, \"d\":4}";
    check(jsmn_parse(&parser, js1, strlen(js1)) == 9);

    tok_index = 0;
    tok = jsmn_token_ref(&parser, tok_index);  // {
    // printf("tok %.*s level = %d\n", jsmn_token_strlen(tok), jsmn_token_string(tok), jsmn_token_level(tok));
    check(jsmn_token_level(tok) == 0);
    check(jsmn_parent_of(&parser, tok_index) == -1);
    check(jsmn_sibling_of(&parser, tok_index) == -1);
    check(jsmn_child_of(&parser, tok_index) == 1); // "a"

    tok_index = 1;
    tok = jsmn_token_ref(&parser, tok_index);  // "a"
    check(jsmn_token_level(tok) == 1);
    check(jsmn_parent_of(&parser, tok_index) == 0); // {
    check(jsmn_sibling_of(&parser, tok_index) == 2); // 1
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 2;
    tok = jsmn_token_ref(&parser, tok_index);  // 1
    check(jsmn_token_level(tok) == 1);
    check(jsmn_parent_of(&parser, tok_index) == 0); // {
    check(jsmn_sibling_of(&parser, tok_index) == 3); // "b"
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 3;
    tok = jsmn_token_ref(&parser, tok_index);  // "b"
    check(jsmn_token_level(tok) == 1);
    check(jsmn_parent_of(&parser, tok_index) == 0); // {
    check(jsmn_sibling_of(&parser, tok_index) == 4); // {
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 4;
    tok = jsmn_token_ref(&parser, tok_index);  // {
    check(jsmn_token_level(tok) == 1);
    check(jsmn_parent_of(&parser, tok_index) == 0); // {
    check(jsmn_sibling_of(&parser, tok_index) == 7); // "d"
    check(jsmn_child_of(&parser, tok_index) == 5);  // "c"

    tok_index = 5;
    tok = jsmn_token_ref(&parser, tok_index);  // "c"
    check(jsmn_token_level(tok) == 2);
    check(jsmn_parent_of(&parser, tok_index) == 4); // {
    check(jsmn_sibling_of(&parser, tok_index) == 6); // 3
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 6;
    tok = jsmn_token_ref(&parser, tok_index);  // 3
    check(jsmn_token_level(tok) == 2);
    check(jsmn_parent_of(&parser, tok_index) == 4); // {
    check(jsmn_sibling_of(&parser, tok_index) == -1);
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 7;
    tok = jsmn_token_ref(&parser, tok_index);  // "d"
    check(jsmn_token_level(tok) == 1);
    check(jsmn_parent_of(&parser, tok_index) == 0); // {
    check(jsmn_sibling_of(&parser, tok_index) == 8); // "d"
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 8;
    tok = jsmn_token_ref(&parser, tok_index);  // 4
    check(jsmn_token_level(tok) == 1);
    check(jsmn_parent_of(&parser, tok_index) == 0); // {
    check(jsmn_sibling_of(&parser, tok_index) == -1);
    check(jsmn_child_of(&parser, tok_index) == -1);  // "c"

    //          index: 0 1   2   3
    const char *js2 = "[10, 20, 30]";
    check(jsmn_parse(&parser, js2, strlen(js2)) == 4);

    tok_index = 0;
    tok = jsmn_token_ref(&parser, tok_index);  // [
    check(jsmn_token_level(tok) == 0);
    check(jsmn_parent_of(&parser, tok_index) == -1);
    check(jsmn_sibling_of(&parser, tok_index) == -1);
    check(jsmn_child_of(&parser, tok_index) == 1); // 10

    tok_index = 1;
    tok = jsmn_token_ref(&parser, tok_index);  // 10
    check(jsmn_token_level(tok) == 1);
    check(jsmn_parent_of(&parser, tok_index) == 0); // [
    check(jsmn_sibling_of(&parser, tok_index) == 2); // 20
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 2;
    tok = jsmn_token_ref(&parser, tok_index);  // 20
    check(jsmn_token_level(tok) == 1);
    check(jsmn_parent_of(&parser, tok_index) == 0); // [
    check(jsmn_sibling_of(&parser, tok_index) == 3); // 30
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 3;
    tok = jsmn_token_ref(&parser, tok_index);  // 30
    check(jsmn_token_level(tok) == 1);
    check(jsmn_parent_of(&parser, tok_index) == 0); // [
    check(jsmn_sibling_of(&parser, tok_index) == -1);
    check(jsmn_child_of(&parser, tok_index) == -1);

    //          index: 0 1  2  3   4    5   6    7
    const char *js3 = "[10, {\"a\":1, \"d\":4}, 30]";
    check(jsmn_parse(&parser, js3, strlen(js3)) == 8);

    tok_index = 0;
    tok = jsmn_token_ref(&parser, tok_index);  // [
    check(jsmn_token_level(tok) == 0);
    check(jsmn_parent_of(&parser, tok_index) == -1);
    check(jsmn_sibling_of(&parser, tok_index) == -1);
    check(jsmn_child_of(&parser, tok_index) == 1); // 10

    tok_index = 1;
    tok = jsmn_token_ref(&parser, tok_index);  // 10
    check(jsmn_token_level(tok) == 1);
    check(jsmn_parent_of(&parser, tok_index) == 0); // [
    check(jsmn_sibling_of(&parser, tok_index) == 2); // {
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 2;
    tok = jsmn_token_ref(&parser, tok_index);  // {
    check(jsmn_token_level(tok) == 1);
    check(jsmn_parent_of(&parser, tok_index) == 0); // [
    check(jsmn_sibling_of(&parser, tok_index) == 7); // 30
    check(jsmn_child_of(&parser, tok_index) == 3); // "a"

    tok_index = 3;
    tok = jsmn_token_ref(&parser, tok_index);  // "a"
    check(jsmn_token_level(tok) == 2);
    check(jsmn_parent_of(&parser, tok_index) == 2); // {
    check(jsmn_sibling_of(&parser, tok_index) == 4); // 1
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 4;
    tok = jsmn_token_ref(&parser, tok_index);  // 1
    check(jsmn_token_level(tok) == 2);
    check(jsmn_parent_of(&parser, tok_index) == 2); // {
    check(jsmn_sibling_of(&parser, tok_index) == 5); // "d"
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 5;
    tok = jsmn_token_ref(&parser, tok_index);  // "d"
    check(jsmn_token_level(tok) == 2);
    check(jsmn_parent_of(&parser, tok_index) == 2); // {
    check(jsmn_sibling_of(&parser, tok_index) == 6); // 4
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 6;
    tok = jsmn_token_ref(&parser, tok_index);  // 4
    check(jsmn_token_level(tok) == 2);
    check(jsmn_parent_of(&parser, tok_index) == 2); // {
    check(jsmn_sibling_of(&parser, tok_index) == -1);
    check(jsmn_child_of(&parser, tok_index) == -1);

    tok_index = 7;
    tok = jsmn_token_ref(&parser, tok_index);  // 30
    check(jsmn_token_level(tok) == 1);
    check(jsmn_parent_of(&parser, tok_index) == 0); // [
    check(jsmn_sibling_of(&parser, tok_index) == -1);
    check(jsmn_child_of(&parser, tok_index) == -1);

    return 0;
}

int test_find_fns(void) {
    jsmn_token_t tokens[15];
    jsmn_parser_t parser;
    int tok_index;
    jsmn_token_t *tok;

    jsmn_init(&parser, tokens, sizeof(tokens) / sizeof(tokens[0]));

    const char *js1 =
        // 0      1        2          3               4
        "{\"jsonrpc\": \"2.0\", \"topic\":\"tstat-logic\", "
        //    5              6           7
        "\"method\": \"relay_set\", \"params\": "
        // 8    9          10         11     12     13   14
        "{\"id\":\"RELAY_Y\", \"value\":true}, \"id\": 5}";
    check(jsmn_parse(&parser, js1, strlen(js1)) == 15);

    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 0), js1));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 1), "jsonrpc"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 2), "2.0"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 3), "topic"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 4), "tstat-logic"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 5), "method"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 6), "relay_set"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 7), "params"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 8), "{\"id\":\"RELAY_Y\", \"value\":true}"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 9), "id"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 10), "RELAY_Y"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 11), "value"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 12), "true"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 13), "id"));
    check(jsmn_token_stringeq(jsmn_token_ref(&parser, 14), "5"));

    check(jsmn_token_find(&parser, "jsonrpc") == 1);
    check(jsmn_token_find(&parser, "2.0") == 2);
    check(jsmn_token_find(&parser, "topic") == 3);
    check(jsmn_token_find(&parser, "tstat-logic") == 4);
    check(jsmn_token_find(&parser, "method") == 5);
    check(jsmn_token_find(&parser, "relay_set") == 6);
    check(jsmn_token_find(&parser, "params") == 7);
    check(jsmn_token_find(&parser, "id") == 9);       // finds first instance
    check(jsmn_token_find(&parser, "RELAY_Y") == 10);
    check(jsmn_token_find(&parser, "value") == 11);
    check(jsmn_token_find(&parser, "true") == 12);
    check(jsmn_token_find(&parser, "id") == 9);       // finds first instance
    check(jsmn_token_find(&parser, "5") == 14);
    check(jsmn_token_find(&parser, "xyzzy") == -1);   // not found

    return 0;
}

int main(void) {
  test(test_empty, "test for a empty JSON objects/arrays");
  test(test_object, "test for a JSON objects");
  test(test_array, "test for a JSON arrays");
  test(test_primitive, "test primitive JSON data types");
  test(test_string, "test string JSON data types");

  test(test_partial_string, "test partial JSON string parsing");
  test(test_partial_array, "test partial array reading");
  test(test_array_nomem, "test array reading with a smaller number of tokens");
  test(test_unquoted_keys, "test unquoted keys (like in JavaScript)");
  test(test_input_length, "test strings that are not null-terminated");
  test(test_issue_22, "test issue #22");
  test(test_issue_27, "test issue #27");
  test(test_count, "test tokens count estimation");
  test(test_nonstrict, "test for non-strict mode");
  test(test_unmatched_brackets, "test for unmatched brackets");
  test(test_object_key, "test for key type");
  test(test_token_types, "test token type predicates");
  test(test_hierarchy, "test hierarchy functions");
  test(test_find_fns, "test find functions");
  printf("\nPASSED: %d\nFAILED: %d\n", test_passed, test_failed);
  return (test_failed > 0);
}
