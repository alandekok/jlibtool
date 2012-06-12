CFLAGS = -g -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -W -Wredundant-decls -Wundef

.PHONY: all clean
all: build-tools libexample.la libexample2.la libexample3.la

clean:
	@rm -f jlibtool  example2.c example3.c CC LINK
	@rm -f *.o *.lo *~
	@rm -f *.la *.so *.dll *.dylib
	@rm -rf .libs/ *.dSYM/

build-tools: jlibtool CC LINK

CC LINK: jlibtool
	@ln -sf $< $@

jlibtool: jlibtool.c
	@$(CC) $< -o $@

######################################################################
#
#  The first example.  You can use jlibtool just like libtool.
#
example.lo: jlibtool example.c
	./jlibtool --mode=compile $(CC) example.c -o $@

libexample.la: example.lo
	./jlibtool --mode=link $(CC) -shared -rpath /usr/local/lib -o $@ $^

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

example2.lo: example2.c jlibtool
	./jlibtool CC -c $< -o $@

libexample2.la: example2.lo jlibtool
	./jlibtool LINK -shared -o $@ $<

######################################################################
#
#  Even better, create a soft link named CC to jlibtool.  It will
#  figure out what to run as above, but with fewer command-line
#  arguments.

example3.c: example.c
	ln -sf $< $@

example3.lo: example3.c CC
	./CC -c $< -o $@

libexample3.la: example3.lo LINK
	./LINK -shared -o $@ $<

#####

example4.c: example.c
	ln -sf $< $@

example4.lo: example4.c CC
	./CC -c $< -o $@

libexample4.a: example4.lo LINK
	./LINK -o $@ $<

libexample4.la: example4.lo jlibtool
	./jlibtool -o $@  $<

main.lo: jlibtool

main: main.lo libexample.la
	./jlibtool -o $@ -rpath /usr/local/lib libexample.la  $< 

main.lo: main.c
	./jlibtool -o $@ -c $<
