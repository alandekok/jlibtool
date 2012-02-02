# About

Jlibtool is a replacement for GNU Libtool.  As it is a C program
rather than a shell script, it is significantly faster than libool.
This speed difference is very noticable on large projects.

## Origin

Jlibtool was originally taken from the Apache Software Foundation, at:

  http://svn.apache.org/repos/asf/apr/apr/trunk/build/jlibtool.c

it was then modified to fix a number of incompatibilities with GNU
Libtool.  It has been used to build a large project (100's of C files,
loadable modules, cross-platform).

## Build

There is no `configure` script, as none is needed.  There is no
`Makefile`, as none is needed.  Jlibtool is written using Posix / C
functions that are available on all modern operating systems.

Instead, just use `make`, which is smart enough to figure out how to
build it:

    $ make

If you don't have `make`, use the following command:

    $ cc jlibtool.c -o jlibtool

If you don't have a working C compiler, don't use jlibtool.

## Portability

It should build without errors on all Posix compatible operating
systems.  It will likely also build on Windows, though that has not
been tried.  If there are build errors on Windows, the patches to fix
the problem are likely to be small.  Please submit patches, and they
will be integrated into the next version.

The source contains compile-time checks for OS/2, Mac OS X, Linux,
FreeBSD, NetBSD, SUN, MingW, and some more esoteric systems.

## Usage

Edit your build system (usually a Makefile), and replace references to
"libtool" with "jlibtool".  Many projects will "just work".

### Even Better Usage

Jlibtool is smarter than libtool in some situations.  For
compatibility, it still accepts the `--mode=compile` options.  But it
can be smarter.  Let's see why, using a common example:

    $ libtool --mode=compile $(CC) $(CFLAGS) -c foo.c -o foo.lo

Stare at that for a second.  Now look at the XKCD comic:

  http://xkcd.com/927/
  
    Panel 1: Situation: There are 14 competing standards.
    
    Panel 2: Ridiculous! We need to develop one universal standard
             that covers everyone's use cases!
    
    Panel 3: Situation: There are 15 competing standards.

There is a better way.

When you compile jlibtool, it knows which C compiler is used.  This
is done by looking at the `CC` environment variable:

    $ CC=gcc gcc jlibtool.c -o jlibtool

The value of `CC` is built into the jlibtool binary.  It can then be
used for subsequent builds.  This means that the redundant
`--mode=compile` text is unnecessary.  Asking the developer to
remember that is rude.  Just use `CC` as the compiler, instead of
`$(CC)`.

    $ ./jlibtool CC -c foo.c -o foo.lo

Even better, make a soft link with a descriptive name:

    $ ln -sf jlibtool CC

The compile command will get even simpler:

    $ ./CC -c foo.c -o foo.lo

That looks a lot better than the libtool versoin.  All of redundant
text of `libtool --mode=compile` is hidden.  It is implied by the
name.  The program figures it out, so that you don't have to remember
it.

This works because jlibtool looks at the name you used to invoke it.
If the name is `CC`, then it acts as a C compiler.  If the name is
`LINK`, it behaves like a (C) linker.  The special names `LINK.c` and
`LINK.cxx` behave as expected, too.

I've switched a number of build systems to using jlibtool.  All of the
platform-specific magic is hidden inside of it.  The build systems
aren't littered with references to `libtool` and hordes of
libtool-specific command-line arguments.  This means your build system
is much simpler, and more understandable.

## Limitations

The sad thing is that jlibtool is _not_ a "drop in" replacement for
libtool.  Libtool has many features, and has been under development
for many years.  Libtool is intended to solve portability problems for
dozens of esoteric operating systems and compilers, many of which no
longer have net access.  The result is that users of libtool are being
punished with slow build times for the mistakes that others made
decades ago.

Jlibtool is intended to solve the "last mile" of portability problems.
That is, for the common case of modern operating systems, it hides the
minor platform-specific details that cause aggravation.

When you want your program to build on multiple systems with minimal
fuss, use jlibtool.  When you want your program to build on systems
that have been running for two decades, use libtool.

## Speed

Since jlibtool is a compiled program, it is enormously faster than
libtool.  Using it in a large project can substantially reduce build
times.

# Install

Don't install it.  Ever.

Installing build tools is a bad idea.

Jlibtool is small enough (~60K) that it can be included in the
distribution archive of your software.  Doing so has the added benefit
that you _know_ the functionality of jlibtool, and that you _know_ it
works.

Many cross-platform build problems with large projects have been
traced to using an installed version of build tools (libtool, libltdl,
etc.), instead of using a version shipped with the project.  The
solution to these problems is to _always_ use local build tools, and
to _never_ install those build tools.

Jlibtool is a tool to help build your project on a variety of
platforms.  It is _not_ a tool to help other people on those platforms
build their projects.  Let them be.  They use their own tools, and
they know what they're doing.

# Compatibility

Jlibtool is _mostly_ compatible with libtool.  It accepts many of
the same command-line arguments as libtool, and behaves largely in
the same way.  See `jlibtool --help` for specific details.

If your project uses libltdl, then jlibtool may not be for you.  There
is a magic relationship between those two programs that jlibtool does
not try to emulate.  If your program requires static linking of
modules, then libtool and libltdl are for you.

However, you should probably change your project to use the normal
`dlopen()` APIs.  As of 2012, all modern Posix systems support the
`dlopen()` API.  Windows does not (of course), but there is a
replacement library available at:

    http://code.google.com/p/dlfcn-win32/

It is smaller than libltdl (~10K versus ~250K), and does not require
integration with libtool.

Similar comments apply to static linking.  There are few reasons any
more to statically link a binary, and have the binary use a `dlopen()`
style API to load modules.  Just use dynamic libaries.

# Issues

Please report any incompatibilities with libtool via the github issues
system, or to the following address:

    Alan DeKok <aland@freeradius.org>

## Author

The original implementation was written by the Apache Foundation.  A
web page is at:

    http://incubator.apache.org/ip-clearance/apr-jlibtool.html

But that hasn't been updated since 2004.

The main Apache Portable Runtime version of jlibtool hasn't had a
commit since July 2010.  I think it's safe to say it's no longer under
active development.

## Changes from upstream

The changes from the upstream version are visible in git.  A short
list is given here:

* Added help text
* print shrext and shrext_cmds for libtool compatibility
* don't dump core if we get --shared for an executable
* add "--mode=execute", which sets LD_LIBRARY_PATH as needed
  * use DYLD_FALLBACK_LIBRARY_PATH on Mac OS X
* Complains descriptively about unknown modes, instead of silently exiting
* Better filename handling
  * Put ".libs/" directory in the correct place, by interpolating it into the filename instead of prefixing it to the filename.
  * "." in "./foo" doesn't signify an extension like it does in "foo.a"
* Use "static" in more places, and remove compiler warnings
* Add --debug parameter to simplify the output
* Added automatic detection of mode via magic programs CC, or magic names CC.

# Libtool and libltld: Just Say No

They were wonderful for their time.  It is time to retire them.
