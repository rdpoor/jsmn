JSMN
====

[![Build Status](https://travis-ci.org/zserge/jsmn.svg?branch=master)](https://travis-ci.org/zserge/jsmn)

jsmn (pronounced like 'jasmine') is a minimalistic JSON parser in C.  It can be
easily integrated into resource-limited or embedded projects.  This version is
based on [Serge Zaitsev's original single-file jsmn.h](https://github.com/zserge/jsmn) with several key differences:
* jsmn is split into two files, jsmn.h and jsmn.c for ease of integration into
workflows that need a two-file layout.
* Each parsed token carries a pointer and a length field to its underlying JSON
string.
* jsmn_token_ref() provides a safe accessor to the parsed tokens.
* A suite of predicates -- jsmn_token_is_array(), jsmn_token_is_number(), etc --
help categorize parsed tokens.

You can find more information about JSON format at [json.org][1]

zserge's original sources are available at https://github.com/zserge/jsmn

The web page with information on the original jsmn design can be found
at [http://zserge.com/jsmn.html][2]

Philosophy
----------

Most JSON parsers offer you a suite of functions to load JSON data, parse it
and extract any value by its name. jsmn takes the view this this is overkill
and instead parses the JSON data into individual token strings, leaving it to
the user to perform the conversion from string to the desired type.

jsmn is designed to be	**robust** (it should work fine even with erroneous
data), **fast** (it should parse data on the fly), **portable** (no superfluous
dependencies or non-standard C extensions). And of course, **simplicity** is a
key feature - simple code style, simple algorithm, simple integration into
other projects.

Features
--------

* compatible with C89
* minimal stdxxx dependencies
* highly portable (tested on x86/amd64, ARM, AVR)
* extremely small code footprint
* no dynamic memory allocation
* incremental single-pass parsing
* (increasingly) complete coverage with unit tests

Design
------

The rudimentary jsmn object is a **token**. Let's consider a JSON string:

	'{ "name" : "Jack", "age" : 27 }'

It holds the following tokens:

* Object: `{ "name" : "Jack", "age" : 27}` (the whole object)
* Strings: `"name"`, `"Jack"`, `"age"` (keys and some values)
* Number: `27`

In jsmn, tokens do not hold parsed data, but point to token boundaries in JSON
string instead. In the example above jsmn will create tokens like:
* Object: `{ "na...`, length = 30
* String: `name" : ...`, length = 4
* String: `Jack", ...`, length = 4
* String: `age" : ...`, length = 3
* Number: `27 }`, length = 2

Every jsmn token has a type, which indicates the type of corresponding JSON
token. jsmn supports the following primitive token types:

* Object - a container of key-value pairs, e.g.:
	`{ "foo":"bar", "x":0.3 }`
* Array - a sequence of values, e.g.:
	`[ 1, 2, 3 ]`
* String - a quoted sequence of chars, e.g.: `"foo"`
* Primitive - a number, a boolean (`true`, `false`) or `null`

Besides start of string and length, jsmn tokens for complex types (like arrays
or objects) also contain the number of child items so you can easily follow
object hierarchy.

This approach provides enough information for parsing any JSON data and makes
it possible to use zero-copy techniques.

Usage
-----

Download `jsmn.h` and `jsmn.c` and incorporate them into your project.

```
#include "jsmn.h"

...
jsmn_parser_t p;
jsmn_token_t t[128]; /* We expect no more than 128 JSON tokens */

jsmn_init(&p, t, 128);
r = jsmn_parse(&p, s, strlen(s)); // "s" is the char array holding the json content
```

API
---

Token types are described by `jsmn_token_type_t`:

	typedef enum {
		JSMN_UNDEFINED = 0,
		JSMN_OBJECT = 1 << 0,
		JSMN_ARRAY = 1 << 1,
		JSMN_STRING = 1 << 2,
		JSMN_PRIMITIVE = 1 << 3
	} jsmn_token_type_t;

**Note:** Unlike JSON data types, primitive tokens are not divided into
numbers, booleans and null, because one can easily tell the type using the
first character:

* `'t', 'f'`- boolean
* `'n'` - null
* `'-', '0'..'9'` - number

Token is an object of `jsmn_token_t` type:

```
typedef struct {
  jsmn_token_type_t type;
  const char *start;     // start of token
  int length;            // length of token
  int size;              // number of nested tokens within OBJECT or ARRAy
#ifdef JSMN_PARENT_LINKS
  int parent;            // index to token that contains this token
#endif
} jsmn_token_t;
```
**Note:** string tokens point to the first character after
the opening quote and the previous symbol before final quote. This was made 
to simplify string extraction from JSON data.

All the parsing is done by a `jsmn_parser_t` object. You can parse a JSON string
as follows:

```
    #include "jsmn.h"

	// you must provide a parser object and an array of tokens
	jsmn_parser_t parser;
	jsmn_token_t tokens[10];

    // initialize the parser and call jsmn_parse() on the string to be parsed.
	jsmn_init(&parser, tokens, sizeof(tokens)/sizeof(jsmn_token_t));
	int r = jsmn_parse(&parser, js, strlen(js));

	if (r < 0) {
		// there was some sort of error.  See JSMN_ERROR_NOMEM, etc, in jsmn.h
	} else {
		// r represents how many tokens were parsed.
	}
```

This will create a parser, and then it tries to parse up to 10 JSON tokens from
the `js` string.

A non-negative return value of `jsmn_parse` is the number of tokens actually
used by the parser.
Passing NULL instead of the tokens array will not store parsing results, but
instead will return the number of tokens needed to parse the given
string. This can be useful if your platforms supports `malloc()` and you don't
yet know how many tokens to allocate.

If something goes wrong, `jsmn_parser()` will return one of these negative
values:

* `JSMN_ERROR_INVAL` - bad token, JSON string is corrupted
* `JSMN_ERROR_NOMEM` - not enough tokens, JSON string is too large
* `JSMN_ERROR_PART` - JSON string is too short, expecting more JSON data


Useful techniques
-----------------
If you get `JSMN_ERROR_NOMEM`, you can re-allocate more tokens and call
`jsmn_parse` once more.

If you read json data from the stream, you can
periodically call `jsmn_parse` and check if return value is `JSMN_ERROR_PART`.
You will get this error until you reach the end of JSON data.

Other info
----------

This software is distributed under [MIT license](http://www.opensource.org/licenses/mit-license.php),
 so feel free to integrate it in your commercial products.

[1]: http://www.json.org/
[2]: http://zserge.com/jsmn.html
