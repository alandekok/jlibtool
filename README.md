# About

Jlibtool is a replacement for GNU Libtool.  As it is a C program
rather than a shell script, it is significantly faster than libtool.
This speed difference is most noticable on large projects.

## Origin

Jlibtool was originally taken from the Apache Software Foundation, at:

  http://svn.apache.org/repos/asf/apr/apr/trunk/build/jlibtool.c

it was then modified to fix a number of incompatibilities with GNU
Libtool.  It has been used to build large projects (100's of C files,
loadable modules, cross-platform).

## Build

There is no `configure` script.  None is needed.  There is no
`Makefile`.  None is needed.  Jlibtool is written using functions that
are available on all modern operating systems.  If your operating
system can't build jlibtool, please submit a patch.

Just use `make`, which is smart enough to figure out how to build C
programs:

    $ make

If you don't have `make`, use the following command:

    $ cc jlibtool.c -o jlibtool

If you do not have a working C compiler, you do not need jlibtool.

## Portability

It should build without errors on all Posix operating systems.  It
will likely also build on Windows, though that has not been tried.  If
there are build errors on Windows, the fixes are likely to be small.
Please submit patches, and they will be integrated into the next
version.

The source contains compile-time checks for OS/2, Mac OS X, Linux,
FreeBSD, NetBSD, SUN, MingW, and some more esoteric systems.  We
believe that these checks are reasonable, and up to date.  If not,
please submit patches.

## Usage

Edit your build system (usually a Makefile), and replace references to
"libtool" with "jlibtool".  Some projects will "just work".

### Libtool Makes Life Harder

For compatibility, jlibtool still accepts the standard libtool options
such as `--mode=compile`.  But jlibtool is smarter than libtool.
Let's see why, using an example.  The following line is typical of how
libtool is used:

    $ libtool --mode=compile $(CC) $(CFLAGS) -c foo.c -o foo.lo

Stare at that for a second.  Now look at the XKCD comic:

  http://xkcd.com/927/
  
    Panel 1: Situation: There are 14 competing standards.
    
    Panel 2: Ridiculous! We need to develop one universal standard
             that covers everyone's use cases!
    
    Panel 3: Situation: There are 15 competing standards.

The "helpful" nature of libtool means that you need to learn _new_
commands and _new_ command-line options.  These options force you to
tell libtool things it already knows.  e.g. when running "CC", you are
really trying to compile a program.

There is a better way.  For simple projects, just do this:

    $ jlibtool -c foo.c -o foo.lo

Jlibtool looks at the input files, the output files, and determines
what to do.  Most of the time this decision can be made automatically.
In those situations, jlibtool makes your life _much_ simpler.

Where you need to pass additional options, you can use them, too:

    $ jlibtool $(CC) $(CFLAGS) -c foo.c -o foo.lo

Or for people who like punishment:

    $ libtool --mode=compile $(CC) $(CFLAGS) -c foo.c -o foo.lo

See the HOWTO.md file in this directory for more complete documentation.

## Limitations

The sad thing is that jlibtool is _not_ a "drop in" replacement for
libtool.  Libtool has many features, and has been under development
for many years.  Libtool is intended to solve portability problems for
dozens of esoteric operating systems and compilers, many of which no
longer have net access.  The result is that users of libtool are being
punished with slow build times for the mistakes that other people made
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
more to statically link a binary, and then use `dlopen()` to load
modules.  Just use dynamic libaries.

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
