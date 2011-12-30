# About

Jlibtool is a replacement for GNU Libtool.  As it is a C program
rather than a shell script, it is significantly faster than libool.
This speed difference is most noticable on large projects.

## Origin

Jlibtool was originally taken from the Apache Software Foundation, at:

	http://svn.apache.org/repos/asf/apr/apr/trunk/build/jlibtool.c

it was then modified to fix a number of incompatibilities with GNU
Libtool.  It has been used to build a large project (100's of C files,
loadable modules, cross-platform).

## Build

There is no `configure` script, as none is needed.  There is no
"Makefile", as none is needed.  It is written using Posix / C
functions that are available on all modern operating systems.

Instead, just use `make`, which is smart enough to figure it out:

    $ make

If you don't have `make`, use the following command:

    $ cc jlibtool.c -o jlibtool

It should build without errors on all Posix compatible operating
systems.  It will likely also build on Windows, though that has not
been tried.  If there are build errors on Windows, the patches to fix
the problem are likely to be small.

## Usage

Edit your build system (usually a Makefile), and replace references to
"libtool" with "jlibtool".  Many projects will "just work".

## Limitations

That being said, jlibtool is not a "drop in" replacement for libtool.
Libtool has many features, and has been under development for many
years.  Libtool is intended to solve portability problems for dozens
of esoteric operating systems and compilers, many of which no longer
have net access.  The result is that users of libtool are being
punished with slow build times for the mistakes of others decades ago.

Jlibtool is intended to solve the "last mile" of portability problems.
That is, for the common case of modern operating systems, it hides the
minor platform-specific details that cause aggravation.

When you want your program to build on multiple systems with minimal
fuss, use jlibtool.  When you want your program to build on systems
you've kept running for a decade, use libtool.

## Speed

As jlibtool is a binary rather than a shell script, it is enormously
faster than libtool.  Using it in a large project can substantially
reduce build times.

# Install

Don't.  Ever.

Installing build tools is a bad idea.

Jlibtool is small enough (~60K) that it can be included in the
distribution archive of your software.  Doing so has the added benefit
that you _know_ the functionality of jlibtool, and that you _know_
that it works.

Many cross-platorm build problems with large projects have been traced
to using an installed version of build tools (libtool, etc.), instead
of using a version shipped with the project.

Jlibtool is a tool to help build your project on a variety of
platforms.  It is _not_ a tool to help other people on those platforms
build their projects.  Let them be.  They use their own tools, and
they know what they're doing.

# Compatibility

Jlibtool is _mostly_ compatible with libtool.  It accepts many of the
same comman-line arguments as jlibtool, and behaves largely in the
same way.  See `jlibtool --help` for specific details.

If your project uses libltdl, then jlibtool is probably not for you.
There is a magic relationship between those two programs that jlibtool
does not try to emulate.

Instead, you can probably change your project to use the normal dlopen
APIs.  As of 2012, all modern Posix systems support dlopen() and
friends.  Windows does not (of course), but there is a replacement
library available at:

	http://code.google.com/p/dlfcn-win32/

It is smaller than libltdl (~10K versus ~250K), and does not require
integration with libtool.

Please report any incompatibilities with libtool via the github issues
system, or to the following address:

       Alan DeKok <aland@freeradius.org>

# Libtool and libltld: Just Say No

