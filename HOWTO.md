# How to use jlibtool

The following examples are taken from the Makefile.

You can use jlibtool just like libtool:

    example.lo: example.c
      ./jlibtool --mode=compile $(CC) example.c -o $@

And then link the library:

    libexample.la: example.lo
      ./jlibtool --mode=link $(CC) -shared -rpath /usr/local/lib -o $@ $^

That works, but it does not show any benefit over using `libtool`.  It
also shows the main problem with using libtool.  In order to create a
"portable" build system, it creates a new build tool, with new
command-line options.  These command-line options are esoteric and
confusing, just like the command-line options they try to hide.

## Simplifying Command-Line Arguments

The command-line arguments can be simplified by deleting the
`--mode=foo $(FOO)`.  It is not rocket science to figure out that
turning a `.c` file into a `.o` file means "compile with `$(CC)`".
Forcing the administrator to explicitly specify that is wasteful.

Build the object file:

    example.lo: example.c
      ./jlibtool -c example.c -o $@

And then link the library:

    libexample.la: example.lo
      ./jlibtool -shared -rpath /usr/local/lib -o $@ $^

That change gets rid of the duplicate command-line arguments needed by
libtool.  The knowledge of which `$(CC)` to use is built into
jlibtool.

The use of built-in configuration can be debated.  We have found that
it is useful in many situations.  More importantly, it does not
_prevent_ you from using the full form of `--mode=FOO $(FOO)`.  One
persons ability to use a simplified version does not infringe on your
freedom to use a more complex version.

The default `CC` can be changed via a command such as:

    $ cc -DCC=i386-mingw32-gcc jlibtool.c -o jlibtool

The built-in definitions are:
: CC - C compiler
: CXX - C++ compiler
: LINK_c - Linker for C programs
: LINK_cxx - Linker for C++ programs
: LINK - Linker for C programs

## Extreme Simplification

That is not all.  The commands above contains duplication which can be
removed.  The "-c" command-line option is not necessary, as the input
file is already a ".c" file.  As fits our philosophy, jlibtool just
Does the Right Thing.

So this works:

    $ ./jlibtool example.c -o example.lo

As does this:

    $ ./jlibtool example.lo -o libexample.la

There is no need to worry about special command-line options.  It Just
Works.

This also works:

    $ ./jlibtool example.lo -o libexample.so

Instead of creating a `.la` file, it just creates a shared library.

## It does not Get In Your Way

In the interest of not getting in your way, these two commands also work:

    $ ./jlibtool example.o -o libexample.so
    $ ./jlibtool example.lo -o libexample.a

i.e. "Use non-PIC object to create a dynamic library", and "use a PIC
object to create a static library".

The underlying philosophy is that the tool should make it easier to do
the Right Thing.  It should _not_ prevent you from doing what you
want.

Why?  You are the administrator, and you should have absolute control
over how you build your programs.  If jlibtool gets in your way, then
you will not use it.  Therefore, it does not get in your way.
