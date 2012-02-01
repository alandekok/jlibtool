CFLAGS = -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -W -Wredundant-decls -Wundef

.PHONY: all clean
all: jlibtool libexample.la libexample2.la libexample3.la

clean:
	rm -f jlibtool *.la *.o *.lo *~ example2.c example3.c CC LINK
	rm -rf .libs

jlibtool: jlibtool.c

example.lo: jlibtool example.c
	./jlibtool --mode=compile $(CC) -c example.c -o $@

libexample.la: example.lo
	./jlibtool --mode=link $(CC) -shared -o $@ $^

######################################################################
#
#  Another example.  This time using the magic "CC" command.
#  This tells jlibtool to run the C compiler which is defined
#  at build time.  It's normally "gcc".  That can be changed by
#  doing:
#
#	$ CC=/path/to/compiler cc jlibtool.c -o jlibtool

example2.c: example.c
	ln -sf $< $@

example2.lo: example2.c
	./jlibtool CC -c example2.c -o $@

libexample2.la: example2.lo
	./jlibtool LINK -shared -o $@ $^

######################################################################
#
#  Even better, create a soft link named CC to jlibtool.  It will
#  figure out what to run as above, but with fewer command-line
#  arguments.

CC LINK: jlibtool
	ln -sf $< $@

example3.c: example.c
	ln -sf $< $@

example3.lo: CC

example3.lo: example3.c
	./CC -c $< -o $@

libexample3.la: LINK

libexample3.la: example3.lo
	./LINK -shared -o $@ $<
