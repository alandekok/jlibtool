CFLAGS = -g -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -W -Wredundant-decls -Wundef

.PHONY: all clean
all: libexample.la libexample2.la main

clean:
	@rm -f jlibtool example2.c example3.c main
	@rm -f *.o *.lo *~
	@rm -f *.la *.so *.dll *.dylib
	@rm -rf .libs/ *.dSYM/

# Create our wonderful program
jlibtool: jlibtool.c
	@$(CC) $< -g -o $@

######################################################################
#
#  The first example.  You can use jlibtool just like libtool.
#
example.lo: example.c jlibtool
	./jlibtool --mode=compile $(CC) example.c -o $@

libexample.la: example.lo jlibtool
	./jlibtool --mode=link $(CC) -shared -rpath /usr/local/lib -o $@ $<

######################################################################
#
#  Another example, where jlibtool Does the Right Thing.  The C
#  compiler is normally "gcc".  That can be changed by doing:
#
#	$ cc -DCC=/path/to/compiler jlibtool.c -o jlibtool

example2.lo: example.c jlibtool
	./jlibtool -c $< -o $@

libexample2.la: example2.lo jlibtool
	./jlibtool -shared -o $@ $<

main.lo: main.c jlibtool
	./jlibtool -o $@ -c $<

main: main.lo libexample.la
	./jlibtool -o $@ -rpath /usr/local/lib libexample.la  $< 
