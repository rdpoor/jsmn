# You can put your build options here
-include config.mk

test: test_default test_strict test_links test_strict_links

test_default: test/tests.c jsmn.c
	$(CC) $(CFLAGS) $(LDFLAGS) $? -o test/$@
	./test/$@

test_strict: test/tests.c jsmn.c
	$(CC) -DJSMN_STRICT=1 $(CFLAGS) $(LDFLAGS) $? -o test/$@
	./test/$@

test_links: test/tests.c jsmn.c
	$(CC) -DJSMN_PARENT_LINKS=1 $(CFLAGS) $(LDFLAGS) $? -o test/$@
	./test/$@

test_strict_links: test/tests.c jsmn.c
	$(CC) -DJSMN_STRICT=1 -DJSMN_PARENT_LINKS=1 $(CFLAGS) $(LDFLAGS) $? -o test/$@
	./test/$@

simple_example: example/simple.c jsmn.c
	$(CC) $(LDFLAGS) $? -o $@

jsondump: example/jsondump.c jsmn.c
	$(CC) $(LDFLAGS) $? -o $@

fmt:
	clang-format -i jsmn.h jsmn.c test/*.[ch] example/*.[ch]

lint:
	clang-tidy jsmn.h --checks='*'

clean:
	rm -f *.o example/*.o
	rm -f simple_example
	rm -f jsondump
	rm -f test/test_default test/test_links test/test_strict test/test_strict_links

.PHONY: clean test

