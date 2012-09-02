# How to use jlibtool

jlibtool is like libtool, but simpler.  The canonical form for running
it is:

    $ ./jlibtool -o output.ext args input.ext ...

Where `output.ext` is the output filename.  Inputs are given by
`input.ext`.  The command-line arguments are given by `args`.  The
decision about what to build is made _automatically_.

    $ ./jlibtool -o foo.lo foo.c

The output is a `lo` file, so it knows to run the C compiler.  The
input is a `c` file, so it knows to run the C compiler.  The output is
a `lo` file, it knows to pass the `-c` option to the C compiler.  It
knows to pass `-fPIC` if the output is a `.lo`, as that will be used
to build a `.la` or `.so`.  It knows to pass `-rdynamic` when building
a `.so`.

The decisions made by jlibtool can rarely go wrong.  In most cases,
there is only one choice for what to do.  And that choice is obvious.
Where the decision might be wrong, you can just specify the move
yourself via `--mode=...`.  jlibtool is smart enough to get things
done for you.  It's also smart enough to get out of your way when you
know better.

The same decisions as above are made for other extensions:

    $ ./jlibtool -o foo ...
    $ ./jlibtool -o foo.a foo.lo bar.lo
    $ ./jlibtool -o libfoo.la foo.lo bar.lo
    $ ./jlibtool -o libfoo.so foo.lo bar.lo
    $ ./jlibtool -o libfoo.dylib foo.lo bar.lo

These commans create (respectively) an executable, a static libary, a
"libtool" dynamic library, a Linux/BSD dynamic library, and finally a
Mac OSX dynamic library.  When used this way, all of redundancy of
`libtool --mode=compile` is avoided.  The operation mode is implied by
the names of the inputs and outputs.  All (or nearly all) of the
command-line option "magic" goes away.  That information is compiled
into jlibtool for your platform.  It remembers the magic so you don't
jabe to.

The goal is for jlibtool to just Do the Right Thing.  All you need to
do is to tell it to "build me a `.so` file", and all of the
platform-specific directives are taken care of.  There is no need to
remember even _more_ magic command-line arguments to a new tool that
"helps" with portability.

Jlibtool has been used in a number of build systems.  All of the
platform-specific magic is hidden inside of the jlibtool binary.  The
build systems aren't littered with references to `libtool` and hordes
of libtool-specific command-line arguments.  This means your build
system is much simpler, and more understandable.

## Passing additional options

The examples given above are extremely simplified.  In some cases, you
need to change the C compiler, or the CFLAGS when building a program.
Doing this is simple:

    $ ./jlibtool $(CC) $(CFLAGS) -o foo.lo foo.c

That's it.

## Using it like libtool

You can use it like libtool, though we don't recommend it.  The
libtool compatibility mode is only for compatibility.  It's not for
simplicity or ease of use.

The following examples are taken from the Makefile.

You can use jlibtool just like libtool:

    example.lo: example.c
      ./jlibtool --mode=compile $(CC) example.c -o $@

And then link the library:

    libexample.la: example.lo
      ./jlibtool --mode=link $(CC) -shared -rpath /usr/local/lib -o $@ $^

That works, but it does not show any benefit over using `libtool`.  It
also shows the main problem with using `libtool`.  In order to create
a "portable" build system, it creates a new build tool, with new
command-line options.  These command-line options are esoteric and
confusing, just like the command-line options they try to hide.

# Build oddities

Like `libtool`, jlibtool uses a `.libs/` directory to store objects.
This is arguably terrible, but it's useful for executing programs from
the build directory, before they have been installed.

# A final word

The underlying philosophy is that the tool should make it easier to do
the Right Thing.  It should _not_ prevent you from doing what you
want.
