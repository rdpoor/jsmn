#ifndef __TEST_UTIL_H__
#define __TEST_UTIL_H__

#include "../jsmn.h"

static int vtokeq(const char *s, jsmn_token_t *t, unsigned long numtok,
                  va_list ap) {
  if (numtok > 0) {
    unsigned long i;
    int start, end, size;
    jsmn_token_type_t type;
    char *value;

    size = -1;
    value = NULL;
    for (i = 0; i < numtok; i++) {
      type = va_arg(ap, jsmn_token_type_t);
      if (type == JSMN_STRING) {
        value = va_arg(ap, char *);
        size = va_arg(ap, int);
        start = end = -1;
      } else if (type == JSMN_PRIMITIVE) {
        value = va_arg(ap, char *);
        start = end = size = -1;
      } else {
        start = va_arg(ap, int);
        end = va_arg(ap, int);
        size = va_arg(ap, int);
        value = NULL;
      }
      if (t[i].type != type) {
        printf("token %lu type is %d, not %d\n", i, t[i].type, type);
        return 0;
      }
      if (start != -1 && end != -1) {
        if ((t[i].start - s) != start) {
          printf("token %lu start is %ld, not %d\n", i, (t[i].start - s), start);
          return 0;
        }
        if ((t[i].start + t[i].length - s) != end) {
          printf("token %lu end is %ld, not %d\n", i, (t[i].start + t[i].length - s), end);
          return 0;
        }
      }
      if (size != -1 && t[i].size != size) {
        printf("token %lu size is %d, not %d\n", i, t[i].size, size);
        return 0;
      }

      if (s != NULL && value != NULL) {
        const char *p = t[i].start;
        if (strlen(value) != (unsigned long)(t[i].length) ||
            strncmp(t[i].start, value, t[i].length) != 0) {
          printf("token %lu value is %.*s, not %s\n", i, t[i].length,
                 t[i].start, value);
          return 0;
        }
      }
    }
  }
  return 1;
}

static int tokeq(const char *s, jsmn_token_t *tokens, unsigned long numtok, ...) {
  int ok;
  va_list args;
  va_start(args, numtok);
  ok = vtokeq(s, tokens, numtok, args);
  va_end(args);
  return ok;
}

static int parse(const char *s, int status, unsigned long numtok, ...) {
  int r;
  int ok = 1;
  va_list args;
  jsmn_parser_t p;
  jsmn_token_t *t = malloc(numtok * sizeof(jsmn_token_t));

  jsmn_init(&p, t, numtok);
  r = jsmn_parse(&p, s, strlen(s));
  if (r != status) {
    printf("status is %d, not %d\n", r, status);
    return 0;
  }

  if (status >= 0) {
    va_start(args, numtok);
    ok = vtokeq(s, t, numtok, args);
    va_end(args);
  }
  free(t);
  return ok;
}

#endif /* __TEST_UTIL_H__ */
