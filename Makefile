CFLAGS = -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -W -Wredundant-decls -Wundef

.PHONY: all clean
all: jlibtool libexample.la

clean:
	rm -f jlibtool libexample.la *.o *.lo *~
	rm -rf .libs

jlibtool: jlibtool.c

example.lo: jlibtool example.c
	@./jlibtool CC -c example.c -o $@

libexample.la: example.lo
	@./jlibtool LINK -shared -o $@ $^
