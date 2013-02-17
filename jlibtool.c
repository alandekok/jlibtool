/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#if !defined(__MINGW32__)
#include <sys/wait.h>
#endif
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>

#ifdef __EMX__
#  define SHELL_CMD  "sh"
#  define GEN_EXPORTS "emxexp"
#  define DEF2IMPLIB_CMD "emximp"
#  define SHARE_SW   "-Zdll -Zmtd"
#  define USE_OMF 1
#  define TRUNCATE_DLL_NAME
#  define DYNAMIC_LIB_EXT "dll"
#  define EXE_EXT ".exe"

#  if USE_OMF
     /* OMF is the native format under OS/2 */
#    define STATIC_LIB_EXT "lib"
#    define OBJECT_EXT     "obj"
#    define LIBRARIAN      "emxomfar"
#    define LIBRARIAN_OPTS "cr"
#  else
     /* but the alternative, a.out, can fork() which is sometimes necessary */
#    define STATIC_LIB_EXT "a"
#    define OBJECT_EXT     "o"
#    define LIBRARIAN      "ar"
#    define LIBRARIAN_OPTS "cr"
#  endif
#endif

#if defined(__APPLE__)
#  define SHELL_CMD  "/bin/sh"
#  define DYNAMIC_LIB_EXT "dylib"
#  define MODULE_LIB_EXT  "bundle"
#  define STATIC_LIB_EXT "a"
#  define OBJECT_EXT     "o"
#  define LIBRARIAN      "ar"
#  define LIBRARIAN_OPTS "cr"
/* man libtool(1) documents ranlib option of -c.  */
#  define RANLIB "ranlib"
#  define PIC_FLAG "-fPIC -fno-common"
#  define SHARED_OPTS "-dynamiclib"
#  define MODULE_OPTS "-bundle -dynamic"
#  define DYNAMIC_LINK_OPTS "-flat_namespace"
#  define DYNAMIC_LINK_UNDEFINED "-undefined suppress"
#  define dynamic_link_version_func darwin_dynamic_link_function
#  define DYNAMIC_INSTALL_NAME "-install_name"
#  define DYNAMIC_LINK_NO_INSTALL "-dylib_file"
#  define HAS_REALPATH
/*-install_name  /Users/jerenk/apache-2.0-cvs/lib/libapr.0.dylib -compatibility_version 1 -current_version 1.0 */
#  define LD_LIBRARY_PATH "DYLD_LIBRARY_PATH"
#  define LD_LIBRARY_PATH_LOCAL "DYLD_FALLBACK_LIBRARY_PATH"
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__)
#  define SHELL_CMD  "/bin/sh"
#  define DYNAMIC_LIB_EXT "so"
#  define MODULE_LIB_EXT  "so"
#  define STATIC_LIB_EXT "a"
#  define OBJECT_EXT     "o"
#  define LIBRARIAN      "ar"
#  define LIBRARIAN_OPTS "cr"
#  define RANLIB "ranlib"
#  define PIC_FLAG "-fPIC"
#  define RPATH "-rpath"
#  define SHARED_OPTS "-shared"
#  define MODULE_OPTS "-shared"
#  define DYNAMIC_LINK_OPTS "-export-dynamic"
#  define LINKER_FLAG_PREFIX "-Wl,"
#  define ADD_MINUS_L
#  define LD_RUN_PATH "LD_RUN_PATH"
#  define LD_LIBRARY_PATH "LD_LIBRARY_PATH"
#  define LD_LIBRARY_PATH_LOCAL LD_LIBRARY_PATH
#endif

#if defined(sun)
#  define SHELL_CMD  "/bin/sh"
#  define DYNAMIC_LIB_EXT "so"
#  define MODULE_LIB_EXT  "so"
#  define STATIC_LIB_EXT "a"
#  define OBJECT_EXT     "o"
#  define LIBRARIAN      "ar"
#  define LIBRARIAN_OPTS "cr"
#  define RANLIB "ranlib"
#  define PIC_FLAG "-KPIC"
#  define RPATH "-R"
#  define SHARED_OPTS "-G"
#  define MODULE_OPTS "-G"
#  define DYNAMIC_LINK_OPTS ""
#  define LINKER_FLAG_NO_EQUALS
#  define ADD_MINUS_L
#  define HAS_REALPATH
#  define LD_RUN_PATH "LD_RUN_PATH"
#  define LD_LIBRARY_PATH "LD_LIBRARY_PATH"
#  define LD_LIBRARY_PATH_LOCAL LD_LIBRARY_PATH
#endif

#if defined(_OSD_POSIX)
#  define SHELL_CMD  "/usr/bin/sh"
#  define DYNAMIC_LIB_EXT "so"
#  define MODULE_LIB_EXT  "so"
#  define STATIC_LIB_EXT "a"
#  define OBJECT_EXT     "o"
#  define LIBRARIAN      "ar"
#  define LIBRARIAN_OPTS "cr"
#  define SHARED_OPTS "-G"
#  define MODULE_OPTS "-G"
#  define LINKER_FLAG_PREFIX "-Wl,"
#  define NEED_SNPRINTF
#endif

#if defined(sinix) && defined(mips) && defined(__SNI_TARG_UNIX)
#  define SHELL_CMD  "/usr/bin/sh"
#  define DYNAMIC_LIB_EXT "so"
#  define MODULE_LIB_EXT  "so"
#  define STATIC_LIB_EXT "a"
#  define OBJECT_EXT     "o"
#  define LIBRARIAN      "ar"
#  define LIBRARIAN_OPTS "cr"
#  define RPATH "-Brpath"
#  define SHARED_OPTS "-G"
#  define MODULE_OPTS "-G"
#  define DYNAMIC_LINK_OPTS "-Wl,-Blargedynsym"
#  define LINKER_FLAG_PREFIX "-Wl,"
#  define NEED_SNPRINTF
#  define LD_RUN_PATH "LD_RUN_PATH"
#  define LD_LIBRARY_PATH "LD_LIBRARY_PATH"
#  define LD_LIBRARY_PATH_LOCAL LD_LIBRARY_PATH
#endif

#if defined(__MINGW32__)
#  define SHELL_CMD "sh"
#  define DYNAMIC_LIB_EXT "dll"
#  define MODULE_LIB_EXT  "dll"
#  define STATIC_LIB_EXT "a"
#  define OBJECT_EXT     "o"
#  define LIBRARIAN      "ar"
#  define LIBRARIAN_OPTS "cr"
#  define RANLIB "ranlib"
#  define LINKER_FLAG_PREFIX "-Wl,"
#  define SHARED_OPTS "-shared"
#  define MODULE_OPTS "-shared"
#  define MKDIR_NO_UMASK
#  define EXE_EXT ".exe"
#endif

#ifndef CC
#define CC "gcc"
#endif

#ifndef CXX
#define CXX "g++"
#endif

#ifndef LINK_c
#define LINK_c "gcc"
#endif

#ifndef LINK_cxx
#define LINK_cxx "g++"
#endif

#ifndef LIBDIR
#define LIBDIR "/usr/local/lib"
#endif

#define OBJDIR ".libs"
#define OBJDIRp OBJDIR "/"

#ifndef SHELL_CMD
#error Unsupported platform: Please add defines for SHELL_CMD etc. for your platform.
#endif

#ifdef NEED_SNPRINTF
#include <stdarg.h>
#endif

#ifdef __EMX__
#include <process.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif


/* We want to say we are libtool 1.4 for shlibtool compatibility. */
#define VERSION "1.4"

enum tool_mode_t {
    mUnknown,
    mCompile,
    mLink,
    mExecute,
    mInstall,
};

enum output_t {
    otUnknown,
    otObject,
    otProgram,
    otLibrary,
    otStaticLibraryOnly,
    otDynamicLibraryOnly,
    otModule,
};

enum input_t {
    inGeneral,
    inObject,
    inCfile
};

enum pic_mode_e {
    pic_UNKNOWN,
    pic_PREFER,
    pic_AVOID,
};

enum shared_mode_e {
    share_UNSET,
    share_STATIC,
    share_SHARED,
};

enum lib_type {
    type_UNKNOWN,
    type_STATIC_LIB,
    type_DYNAMIC_LIB,
    type_MODULE_LIB,
    type_OBJECT,
};

typedef struct {
    const char **vals;
    int num;
} count_chars;

typedef struct {
    const char *normal;
    const char *install;
} library_name;

typedef struct {
    count_chars *normal;
    count_chars *install;
    count_chars *dependencies;
} library_opts;

typedef struct {
    int silent;
    int debug;
    enum shared_mode_e shared;
    int export_all;
    int dry_run;
    enum pic_mode_e pic_mode;
    int export_dynamic;
    int no_install;
} options_t;

typedef struct {
    enum tool_mode_t mode;
    enum output_t output;
    enum input_t input;
    options_t options;

    char *output_name;
    const char *fake_output_name;
    char *basename;

    const char *install_path;
    const char *compiler;
    const char *program;
    count_chars *program_opts;

    count_chars *arglist;
    count_chars *tmp_dirs;
    count_chars *obj_files;
    count_chars *dep_rpaths;
    count_chars *rpaths;

    library_name static_name;
    library_name shared_name;
    library_name module_name;

    library_opts static_opts;
    library_opts shared_opts;

    const char *version_info;
    const char *undefined_flag;
} command_t;

static const char *progname = NULL;

#ifdef RPATH
static void add_rpath(count_chars *cc, const char *path);
#endif

static void usage(int code)
{
	printf("Usage: jlibtool [OPTIONS...] COMMANDS...\n");
	printf("jlibtool is a replacement for GNU libtool with similar functionality.\n\n");

	printf("    --config          show all configuration variables\n");
	printf("                      use --config=FOO to show variable FOO\n");
	printf("    --debug           enable verbose shell tracing\n");
	printf("    --dry-run         display commands without modifying any files\n");
	printf("    --help            display this help message and exit\n");
	printf("    --mode=MODE       use operational mode MODE (you *must* set mode)\n");

	printf("    --silent          don't print informational messages\n");
	printf("    --tag=TAG         Ignored for libtool compatibility\n");
	printf("    --version         print version information\n");


	printf("    --shared          Build shared libraries when using --mode=link\n");
	printf("    --export-all      Try to export 'def' file on some platforms\n");

	printf("\nMODE must be one of the following:\n\n");
	printf("      compile         compile a source file into a jlibtool object\n");
	printf("      execute         automatically set library path, then run a program\n");
        printf("      install         install libraries or executables\n");
        printf("      link            create a library or an executable\n");

	printf("\nMODE-ARGS can be the following:\n\n");
	printf("      -export-dynamic  accepted and ignored\n");
	printf("      -module          create a module when linking\n");
	printf("      -shared          create a shared library when linking\n");
	printf("      -prefer-pic      prefer position-independent-code when compiling\n");
	printf("      -prefer-non-pic  prefer non position-independent-code when compiling\n");
	printf("      -static          create a static library when linking\n");
	printf("      -no-install      link libraries locally\n");
	printf("      -rpath arg       Set install path for shared libraries\n");
	printf("      -l arg           pass '-l arg' to the link stage\n");
	printf("      -L arg           pass '-L arg' to the link stage\n");
	printf("      -R dir           add 'dir' to runtime library search path.\n");
	printf("      -Zexe            accepted and ignored\n");
	printf("      -avoid-version   accepted and ignored\n");

	exit(code);
}

#if defined(NEED_SNPRINTF)
/* Write at most n characters to the buffer in str, return the
 * number of chars written or -1 if the buffer would have been
 * overflowed.
 *
 * This is portable to any POSIX-compliant system has /dev/null
 */
static FILE *f=NULL;
static int vsnprintf( char *str, size_t n, const char *fmt, va_list ap )
{
       int res;

       if (f == NULL)
               f = fopen("/dev/null","w");
       if (f == NULL)
               return -1;

       setvbuf( f, str, _IOFBF, n );

       res = vfprintf( f, fmt, ap );

       if ( res > 0 && res < n ) {
               res = vsprintf( str, fmt, ap );
       }
       return res;
}
static int snprintf( char *str, size_t n, const char *fmt, ... )
{
        va_list ap;
        int res;

        va_start( ap, fmt );
        res = vsnprintf( str, n, fmt, ap );
        va_end( ap );
        return res;
}
#endif

static void init_count_chars(count_chars *cc)
{
    cc->vals = (const char**)malloc(PATH_MAX*sizeof(char*));
    cc->num = 0;
}

static void clear_count_chars(count_chars *cc)
{
    int i;
    for (i = 0; i < cc->num; i++) {
        cc->vals[i] = 0;
    }

    cc->num = 0;
}

static void push_count_chars(count_chars *cc, const char *newval)
{
    cc->vals[cc->num++] = newval;
}

static void pop_count_chars(count_chars *cc)
{
    cc->num--;
}

static void insert_count_chars(count_chars *cc, const char *newval, int position)
{
    int i;

    for (i = cc->num; i > position; i--) {
        cc->vals[i] = cc->vals[i-1];
    }

    cc->vals[position] = newval;
    cc->num++;
}

static void append_count_chars(count_chars *cc, count_chars *cctoadd)
{
    int i;
    for (i = 0; i < cctoadd->num; i++) {
        if (cctoadd->vals[i]) {
            push_count_chars(cc, cctoadd->vals[i]);
        }
    }
}

static const char *flatten_count_chars(count_chars *cc, int space)
{
    int i, size;
    char *newval;

    size = 0;
    for (i = 0; i < cc->num; i++) {
        if (cc->vals[i]) {
            size += strlen(cc->vals[i]) + 1;
            if (space) {
              size++;
            }
        }
    }

    newval = (char*)malloc(size + 1);
    newval[0] = 0;

    for (i = 0; i < cc->num; i++) {
        if (cc->vals[i]) {
            strcat(newval, cc->vals[i]);
            if (space) {
                strcat(newval, " ");
            }
        }
    }

    return newval;
}

static char *shell_esc(const char *str)
{
    int in_quote = 0;
    char *cmd;
    unsigned char *d;
    const unsigned char *s;

    cmd = (char *)malloc(2 * strlen(str) + 3);
    d = (unsigned char *)cmd;
    s = (const unsigned char *)str;

#ifdef __MINGW32__
    *d++ = '\"';
#endif

    for (; *s; ++s) {
        if (*s == '"') {
            *d++ = '\\';
            in_quote++;
        }
        else if (*s == '\\' || (*s == ' ' && (in_quote % 2))) {
            *d++ = '\\';
        }
        *d++ = *s;
    }

#ifdef __MINGW32__
    *d++ = '\"';
#endif

    *d = '\0';
    return cmd;
}

static int external_spawn(command_t *cmd, const char *file, const char **argv)
{
    file = file;		/* -Wunused */

    if (!cmd->options.silent) {
        const char **argument = argv;
        printf("Executing: ");
        while (*argument) {
            printf("%s ", *argument);
            argument++;
        }
        puts("");
    }

    if (cmd->options.dry_run) {
        return 0;
    }
#if defined(__EMX__) || defined(__MINGW32__)
    return spawnvp(P_WAIT, argv[0], argv);
#else
    {
        pid_t pid;
        pid = fork();
        if (pid == 0) {
            return execvp(argv[0], (char**)argv);
        }
        else {
            int statuscode;
            waitpid(pid, &statuscode, 0);
            if (WIFEXITED(statuscode)) {
                return WEXITSTATUS(statuscode);
            }
            return 0;
        }
    }
#endif
}

static int run_command(command_t *cmd_data, count_chars *cc)
{
    char *command;
    const char *spawn_args[4];
    count_chars tmpcc;

    init_count_chars(&tmpcc);

    if (cmd_data->program) {
        push_count_chars(&tmpcc, cmd_data->program);
    }

    append_count_chars(&tmpcc, cmd_data->program_opts);

    append_count_chars(&tmpcc, cc);

    command = shell_esc(flatten_count_chars(&tmpcc, 1));

    spawn_args[0] = SHELL_CMD;
    spawn_args[1] = "-c";
    spawn_args[2] = command;
    spawn_args[3] = NULL;
    return external_spawn(cmd_data, spawn_args[0], spawn_args);
}

/*
 * print configuration
 * shlibpath_var is used in configure.
 */
#define printc(_x,_y) if (!value || !*value || !strcmp(value, _x)) printf(_x "=\"%s\"\n", _y)

static void print_config(const char *value)
{
#ifdef LD_RUN_PATH
    printc("runpath_var", LD_RUN_PATH);
#endif
#ifdef LD_LIBRARY_PATH
    printc("shlibpath_var", LD_LIBRARY_PATH);
#endif
#ifdef LD_LIBRARY_PATH_LOCAL
    printc("shlocallibpath_var", LD_LIBRARY_PATH_LOCAL);
#endif
#ifdef SHELL_CMD
    printc("SHELL", SHELL_CMD);
#endif
#ifdef OBJECT_EXT
    printc("objext", OBJECT_EXT);
#endif
#ifdef OBJDIR
    printc("objdir", OBJDIR);
#endif
#ifdef DYNAMIC_LIB_EXT
    /* add a '.' prefix because libtool does that. */
    printc("shrext_cmds", "echo ." DYNAMIC_LIB_EXT);
    /* add a '.' prefix because libtool does that. */
    printc("shrext", "." DYNAMIC_LIB_EXT);
#endif
#ifdef EXE_EXT
    printc("exeext", EXE_EXT);
#endif
#ifdef STATIC_LIB_EXT
    printc("libext", STATIC_LIB_EXT);
#endif
#ifdef LIBRARIAN
    printc("AR", LIBRARIAN);
#endif
#ifdef LIBRARIAN_OPTS
    printc("AR_FLAGS", LIBRARIAN_OPTS);
#endif
#ifdef LINKER_FLAG_PREFIX
    printc("wl", LINKER_FLAG_PREFIX);
#endif
#ifdef RANLIB
    printc("ranlib", RANLIB);
#endif
    printc("CC", CC);
    printc("CXX", CXX);
    printc("LINK_c", LINK_c);
    printc("LINK_cxx", LINK_cxx);
    printc("LIBDIR", LIBDIR);
}
/*
 * Add a directory to the runtime library search path.
 */
static void add_runtimedirlib(char *arg, command_t *cmd_data)
{
#ifdef RPATH
    add_rpath(cmd_data->shared_opts.dependencies, arg);
#else
    arg = arg;			/* -Wunused */
    cmd_data = cmd_data;
#endif
}

static int parse_long_opt(char *arg, command_t *cmd_data)
{
    char *equal_pos = strchr(arg, '=');
    char var[50];
    char value[500];

    if (equal_pos) {
        strncpy(var, arg, equal_pos - arg);
        var[equal_pos - arg] = 0;
        strcpy(value, equal_pos + 1);
    } else {
        strcpy(var, arg);
	value[0] = '\0';
    }

    if (strcmp(var, "silent") == 0) {
        cmd_data->options.silent = 1;
    } else if (strcmp(var, "debug") == 0) {
        cmd_data->options.debug = 1;
    } else if (strcmp(var, "mode") == 0) {
	if (cmd_data->mode != mUnknown) {
		fprintf(stderr, "Cannot set --mode twice\n");
		exit(1);
	}

        if (strcmp(value, "compile") == 0) {
            cmd_data->mode = mCompile;
            cmd_data->output = otObject;

        } else if (strcmp(value, "link") == 0) {
            cmd_data->mode = mLink;
            cmd_data->output = otLibrary;

	} else if (strcmp(value, "install") == 0) {
            cmd_data->mode = mInstall;

	} else if (strcmp(value, "execute") == 0) {
            cmd_data->mode = mExecute;

        } else {
	    printf("Unknown mode \"%s\"\n", value);
	}
    } else if (strcmp(var, "shared") == 0) {
      if ((cmd_data->mode == mLink) &&
	  (cmd_data->output == otUnknown)) {
            cmd_data->output = otDynamicLibraryOnly;
	}
        cmd_data->options.shared = share_SHARED;
    } else if (strcmp(var, "export-all") == 0) {
        cmd_data->options.export_all = 1;
    } else if (strcmp(var, "dry-run") == 0) {
        printf("Dry-run mode on!\n");
        cmd_data->options.dry_run = 1;
    } else if (strcmp(var, "version") == 0) {
        printf("Version " VERSION "\n");
    } else if (strcmp(var, "help") == 0) {
	usage(0);
    } else if (strcmp(var, "config") == 0) {
        print_config(value);
	exit(0);
    } else if (strcmp(var, "tag") == 0) {
        if (strcmp(value, "CC") == 0) {
            /* Do nothing. */
        }
        if (strcmp(value, "CXX") == 0) {
            /* Do nothing. */
        }
    } else {
        return 0;
    }

    return 1;
}

/* Return 1 if we eat it. */
static int parse_short_opt(char *arg, command_t *cmd_data)
{
    if (strcmp(arg, "export-dynamic") == 0) {
        cmd_data->options.export_dynamic = 1;
        return 1;
    }

    if (strcmp(arg, "module") == 0) {
        cmd_data->output = otModule;
        return 1;
    }

    if (strcmp(arg, "shared") == 0) {
        if (cmd_data->mode == mLink) {
            cmd_data->output = otDynamicLibraryOnly;
        }
        cmd_data->options.shared = share_SHARED;
        return 1;
    }

    if (strcmp(arg, "Zexe") == 0) {
        return 1;
    }

    if (strcmp(arg, "avoid-version") == 0) {
        return 1;
    }

    if (strcmp(arg, "prefer-pic") == 0) {
        cmd_data->options.pic_mode = pic_PREFER;
        return 1;
    }

    if (strcmp(arg, "prefer-non-pic") == 0) {
        cmd_data->options.pic_mode = pic_AVOID;
        return 1;
    }

    if (strcmp(arg, "static") == 0) {
       if ((cmd_data->mode == mLink) &&
	  (cmd_data->output == otLibrary)) {
            cmd_data->output = otStaticLibraryOnly;
	}
        cmd_data->options.shared = share_STATIC;
        return 1;
    }

    if (cmd_data->mode == mLink) {
        if (strcmp(arg, "no-install") == 0) {
            cmd_data->options.no_install = 1;
            return 1;
        }
        if (arg[0] == 'L' || arg[0] == 'l') {
            /* Hack... */
            arg--;
            push_count_chars(cmd_data->shared_opts.dependencies, arg);
            return 1;
        } else if (arg[0] == 'R' && arg[1]) {
            /* -Rdir Add dir to runtime library search path. */
            add_runtimedirlib(&arg[1], cmd_data);
            return 1;
        }
    }
    return 0;
}

#ifdef TRUNCATE_DLL_NAME
static char *truncate_dll_name(char *path)
{
    /* Cut DLL name down to 8 characters after removing any mod_ prefix */
    char *tmppath = strdup(path);
    char *newname = strrchr(tmppath, '/') + 1;
    char *ext = strrchr(newname, '.');
    int len;

    if (ext == NULL)
        return tmppath;

    len = ext - newname;

    if (strncmp(newname, "mod_", 4) == 0) {
        strcpy(newname, newname + 4);
        len -= 4;
    }

    if (len > 8) {
        strcpy(newname + 8, strchr(newname, '.'));
    }

    return tmppath;
}
#endif

static long safe_strtol(const char *nptr, const char **endptr, int base)
{
    long rv;

    errno = 0;

    rv = strtol(nptr, (char**)endptr, 10);

    if (errno == ERANGE) {
        return 0;
    }

    return rv;
}

static void safe_mkdir(const char *path)
{
    int status;
    mode_t old_umask;

    old_umask = umask(0);
    umask(old_umask);

#ifdef MKDIR_NO_UMASK
    status = mkdir(path);
#else
    status = mkdir(path, ~old_umask);
#endif
    if (status < 0) {
        printf("Warning: mkdir of %s failed\n", path);
    }
}

/* returns just a file's name without the path */
static const char *jlibtool_basename(const char *fullpath)
{
    const char *name = strrchr(fullpath, '/');

    if (name == NULL) {
        name = strrchr(fullpath, '\\');
    }

    if (name == NULL) {
        name = fullpath;
    } else {
        name++;
    }

    return name;
}

#ifdef GEN_EXPORTS
/* returns just a file's name without path or extension */
static const char *nameof(const char *fullpath)
{
    const char *name;
    const char *ext;

    name = jlibtool_basename(fullpath);
    ext = strrchr(name, '.');

    if (ext) {
        char *trimmed;
        trimmed = malloc(ext - name + 1);
        strncpy(trimmed, name, ext - name);
        trimmed[ext-name] = 0;
        return trimmed;
    }

    return name;
}
#endif

/* version_info is in the form of MAJOR:MINOR:PATCH */
static const char *darwin_dynamic_link_function(const char *version_info)
{
    char *newarg;
    long major, minor, patch;

    major = 0;
    minor = 0;
    patch = 0;

    if (version_info) {
        major = safe_strtol(version_info, &version_info, 10);

        if (version_info) {
            if (version_info[0] == ':') {
                version_info++;
            }

            minor = safe_strtol(version_info, &version_info, 10);

            if (version_info) {
                if (version_info[0] == ':') {
                    version_info++;
                }

                patch = safe_strtol(version_info, &version_info, 10);

            }
        }
    }

    /* Avoid -dylib_compatibility_version must be greater than zero errors. */
    if (major == 0) {
        major = 1;
    }
    newarg = (char*)malloc(100);
    snprintf(newarg, 99,
             "-compatibility_version %ld -current_version %ld.%ld",
             major, major, minor);

    return newarg;
}


/*
 *	Insert a '.libs/' to the buffer.  The caller ensures that
 *	The buffer is large enough to handle 6 extra characters.
 *
 *	foo/bar/baz.o --> foo/bar/.libs/baz.o
 *	baz.o --> .libs/baz.o
 */
static void add_dotlibs(char *buffer)
{
	char *name;
	const char *base;
	
	if (!buffer[0]) {
		strcpy(buffer, OBJDIRp);
		return;
	}
	
	base = jlibtool_basename(buffer);
	name = buffer + (base - buffer); /* really name = base */

	memmove(name + sizeof(OBJDIRp) - 1, name, strlen(name));
	memcpy(name, OBJDIRp, sizeof(OBJDIRp) - 1);
}

static char *gen_library_name(const char *name, enum lib_type genlib)
{
    char *newarg, *newext;

    newarg = (char *)calloc(strlen(name) + 11, 1);

    if (genlib == type_MODULE_LIB && strncmp(name, "lib", 3) == 0) {
        name += 3;
    }

    if (genlib == type_MODULE_LIB) {
        strcpy(newarg, jlibtool_basename(name));
    }
    else {
        strcpy(newarg, name);
    }

    newext = strrchr(newarg, '.') + 1;

    switch (genlib) {
    case type_STATIC_LIB:
        strcpy(newext, STATIC_LIB_EXT);
        break;
    case type_DYNAMIC_LIB:
        strcpy(newext, DYNAMIC_LIB_EXT);
        break;
    case type_MODULE_LIB:
        strcpy(newext, MODULE_LIB_EXT);
        break;

    default:
        break;
    }

    add_dotlibs(newarg);

    return newarg;
}

static char *gen_install_name(const char *name, enum lib_type genlib)
{
    struct stat sb;
    char *newname;
    int rv;

    newname = gen_library_name(name, genlib);

    /* Check if it exists. If not, return NULL.  */
    rv = stat(newname, &sb);

    if (rv) {
        return NULL;
    }

    return newname;
}

static const char *check_object_exists(command_t *cmd, const char *arg, int arglen)
{
    char *newarg, *ext;
    int pass, rv;

    newarg = (char *)malloc(arglen + 10);
    memcpy(newarg, arg, arglen);
    newarg[arglen] = 0;
    ext = newarg + arglen;

    pass = 0;

    do {
        struct stat sb;

        switch (pass) {
        case 0:
            strcpy(ext, OBJECT_EXT);
            break;
/*
        case 1:
            strcpy(ext, NO_PIC_EXT);
            break;
*/
        default:
            break;
        }

        if (cmd->options.debug) {
            printf("Checking (obj): %s\n", newarg);
        }
        rv = stat(newarg, &sb);
    }
    while (rv != 0 && ++pass < 1);

    if (rv == 0) {
        if (pass == 1) {
            cmd->options.pic_mode = pic_AVOID;
        }
        return newarg;
    }

    return NULL;
}

/* libdircheck values:
 * 0 - no .libs suffix
 * 1 - .libs suffix
 */
static char *check_library_exists(command_t *cmd, const char *arg, int pathlen,
                           int libdircheck, enum lib_type *libtype)
{
    char *newarg, *ext;
    int rv, newpathlen;
    enum lib_type pass;

    newarg = (char *)malloc(strlen(arg) + 10);
    strcpy(newarg, arg);
    newarg[pathlen] = 0;

    newpathlen = pathlen;
    if (libdircheck) {
        add_dotlibs(newarg);
        newpathlen += sizeof(OBJDIRp) - 1;
    }

    strcpy(newarg+newpathlen, arg+pathlen);
    ext = strrchr(newarg, '.') + 1;

    pass = type_UNKNOWN;

    do {
        struct stat sb;

        switch (pass) {
        case type_UNKNOWN:
            if (cmd->options.pic_mode != pic_AVOID &&
                cmd->options.shared != share_STATIC) {
                strcpy(ext, DYNAMIC_LIB_EXT);
                *libtype = type_DYNAMIC_LIB;
                break;
            }
            pass = type_STATIC_LIB;
            /* Fall through */
        case type_STATIC_LIB:
            strcpy(ext, STATIC_LIB_EXT);
            *libtype = pass;
            break;
        case type_MODULE_LIB:
            strcpy(ext, MODULE_LIB_EXT);
            *libtype = pass;
            break;
        case type_OBJECT:
            strcpy(ext, OBJECT_EXT);
            *libtype = pass;
            break;
        default:
            *libtype = type_UNKNOWN;
            break;
        }

        if (cmd->options.debug) {
            printf("Checking (lib): %s\n", newarg);
        }
        rv = stat(newarg, &sb);
    }
    while (rv != 0 && ++pass < 4);

    if (rv == 0) {
        return newarg;
    }

    return NULL;
}

static char * load_install_path(const char *arg)
{
    FILE *f;
    char *path;

    path = malloc(PATH_MAX);

    f = fopen(arg,"r");
    if (f == NULL) {
        return NULL;
    }
    fgets(path, PATH_MAX, f);
    fclose(f);
    if (path[strlen(path)-1] == '\n') {
        path[strlen(path)-1] = '\0';
    }
    /* Check that we have an absolute path.
     * Otherwise the file could be a GNU libtool file.
     */
    if (path[0] != '/') {
        return NULL;
    }
    return path;
}

static char * load_noinstall_path(const char *arg, int pathlen)
{
    char *newarg, *expanded_path;
    int newpathlen;

    newarg = (char *)malloc(strlen(arg) + 10);
    strcpy(newarg, arg);
    newarg[pathlen] = 0;

    newpathlen = pathlen;
    strcat(newarg, OBJDIR);
    newpathlen += sizeof(OBJDIR) - 1;
    newarg[newpathlen] = 0;

#ifdef HAS_REALPATH
    expanded_path = malloc(PATH_MAX);
    expanded_path = realpath(newarg, expanded_path);
    /* Uh, oh.  There was an error.  Fall back on our first guess. */
    if (!expanded_path) {
        expanded_path = newarg;
    }
#else
    /* We might get ../ or something goofy.  Oh, well. */
    expanded_path = newarg;
#endif

    return expanded_path;
}

static void add_dynamic_link_opts(command_t *cmd_data, count_chars *args)
{
#ifdef DYNAMIC_LINK_OPTS
    if (cmd_data->options.pic_mode != pic_AVOID) {
        if (cmd_data->options.debug) {
           printf("Adding: %s\n", DYNAMIC_LINK_OPTS);
        }
        push_count_chars(args, DYNAMIC_LINK_OPTS);
        if (cmd_data->undefined_flag) {
            push_count_chars(args, "-undefined");
#if defined(__APPLE__)
            /* -undefined dynamic_lookup is used by the bundled Python in
             * 10.4, but if we don't set MACOSX_DEPLOYMENT_TARGET to 10.3+,
             * we'll get a linker error if we pass this flag.
             */
            if (strcasecmp(cmd_data->undefined_flag,
                           "dynamic_lookup") == 0) {
                insert_count_chars(cmd_data->program_opts,
                                   "MACOSX_DEPLOYMENT_TARGET=10.3", 0);
            }
#endif
            push_count_chars(args, cmd_data->undefined_flag);
        }
        else {
#ifdef DYNAMIC_LINK_UNDEFINED
            if (cmd_data->options.debug) {
                printf("Adding: %s\n", DYNAMIC_LINK_UNDEFINED);
            }
            push_count_chars(args, DYNAMIC_LINK_UNDEFINED);
#endif
        }
    }
#endif
}

/* Read the final install location and add it to runtime library search path. */
#ifdef RPATH
static void add_rpath(count_chars *cc, const char *path)
{
    int size = 0;
    char *tmp;

#ifdef LINKER_FLAG_PREFIX
    size = strlen(LINKER_FLAG_PREFIX);
#endif
    size = size + strlen(path) + strlen(RPATH) + 2;
    tmp = malloc(size);
    if (tmp == NULL) {
        return;
    }
#ifdef LINKER_FLAG_PREFIX
    strcpy(tmp, LINKER_FLAG_PREFIX);
    strcat(tmp, RPATH);
#else
    strcpy(tmp, RPATH);
#endif
#ifndef LINKER_FLAG_NO_EQUALS
    strcat(tmp, "=");
#endif
    strcat(tmp, path);

    push_count_chars(cc, tmp);
}

static void add_rpath_file(count_chars *cc, const char *arg)
{
    const char *path;

    path = load_install_path(arg);
    if (path) {
        add_rpath(cc, path);
    }
}

static void add_rpath_noinstall(count_chars *cc, const char *arg, int pathlen)
{
    const char *path;

    path = load_noinstall_path(arg, pathlen);
    if (path) {
        add_rpath(cc, path);
    }
}
#endif

#ifdef DYNAMIC_LINK_NO_INSTALL
static void add_dylink_noinstall(count_chars *cc, const char *arg, int pathlen,
                          int extlen)
{
    const char *install_path, *current_path, *name;
    char *exp_argument;
    int i_p_len, c_p_len, name_len, dyext_len, cur_len;

    install_path = load_install_path(arg);
    current_path = load_noinstall_path(arg, pathlen);

    if (!install_path || !current_path) {
        return;
    }

    push_count_chars(cc, DYNAMIC_LINK_NO_INSTALL);

    i_p_len = strlen(install_path);
    c_p_len = strlen(current_path);

    name = arg+pathlen;
    name_len = extlen-pathlen;
    dyext_len = sizeof(DYNAMIC_LIB_EXT) - 1;

    /* No, we need to replace the extension. */
    exp_argument = (char *)malloc(i_p_len + c_p_len + (name_len*2) +
                                  (dyext_len*2) + 2);

    cur_len = 0;
    strcpy(exp_argument, install_path);
    cur_len += i_p_len;
    exp_argument[cur_len++] = '/';
    strncpy(exp_argument+cur_len, name, extlen-pathlen);
    cur_len += name_len;
    strcpy(exp_argument+cur_len, DYNAMIC_LIB_EXT);
    cur_len += dyext_len;
    exp_argument[cur_len++] = ':';
    strcpy(exp_argument+cur_len, current_path);
    cur_len += c_p_len;
    exp_argument[cur_len++] = '/';
    strncpy(exp_argument+cur_len, name, extlen-pathlen);
    cur_len += name_len;
    strcpy(exp_argument+cur_len, DYNAMIC_LIB_EXT);
    cur_len += dyext_len;

    push_count_chars(cc, exp_argument);
}
#endif

#ifdef ADD_MINUS_L
/* use -L -llibname to allow to use installed libraries */
static void add_minus_l(count_chars *cc, const char *arg)
{
    char *newarg;
    char *name = strrchr(arg, '/');
    char *file = strrchr(arg, '.');
    char *lib  = strstr(name, "lib");

    if (name !=NULL && file != NULL && lib == name+1) {
        *name = '\0';
        *file = '\0';
        file = name;
        file = file+4;
        push_count_chars(cc, "-L");
        push_count_chars(cc, arg);
        /* we need one argument like -lapr-1 */
        newarg = malloc(strlen(file) + 3);
        strcpy(newarg, "-l");
        strcat(newarg, file);
        push_count_chars(cc, newarg);
    } else {
        push_count_chars(cc, arg);
    }
}
#endif

#if 0
static void add_linker_flag_prefix(count_chars *cc, const char *arg)
{
#ifndef LINKER_FLAG_PREFIX
    push_count_chars(cc, arg);
#else
    char *newarg;
    newarg = (char*)malloc(strlen(arg) + sizeof(LINKER_FLAG_PREFIX) + 1);
    strcpy(newarg, LINKER_FLAG_PREFIX);
    strcat(newarg, arg);
    push_count_chars(cc, newarg);
#endif
}
#endif

static int explode_static_lib(command_t *cmd_data, const char *lib)
{
    count_chars tmpdir_cc, libname_cc;
    const char *tmpdir, *libname;
    char savewd[PATH_MAX];
    const char *name;
    DIR *dir;
    struct dirent *entry;
    const char *lib_args[4];

    /* Bah! */
    if (cmd_data->options.dry_run) {
        return 0;
    }

    name = jlibtool_basename(lib);

    init_count_chars(&tmpdir_cc);
    push_count_chars(&tmpdir_cc, OBJDIRp);
    push_count_chars(&tmpdir_cc, name);
    push_count_chars(&tmpdir_cc, ".exploded/");
    tmpdir = flatten_count_chars(&tmpdir_cc, 0);

    if (!cmd_data->options.silent) {
        printf("Making: %s\n", tmpdir);
    }
    safe_mkdir(tmpdir);

    push_count_chars(cmd_data->tmp_dirs, tmpdir);

    getcwd(savewd, sizeof(savewd));

    if (chdir(tmpdir) != 0) {
        if (!cmd_data->options.silent) {
            printf("Warning: could not explode %s\n", lib);
        }
        return 1;
    }

    if (lib[0] == '/') {
        libname = lib;
    }
    else {
        init_count_chars(&libname_cc);
        push_count_chars(&libname_cc, "../../");
        push_count_chars(&libname_cc, lib);
        libname = flatten_count_chars(&libname_cc, 0);
    }

    lib_args[0] = LIBRARIAN;
    lib_args[1] = "x";
    lib_args[2] = libname;
    lib_args[3] = NULL;

    external_spawn(cmd_data, LIBRARIAN, lib_args);

    chdir(savewd);
    dir = opendir(tmpdir);

    while ((entry = readdir(dir)) != NULL) {
#if defined(__APPLE__) && defined(RANLIB)
        /* Apple inserts __.SYMDEF which isn't needed.
         * Leopard (10.5+) can also add '__.SYMDEF SORTED' which isn't
         * much fun either.  Just skip them.
         */
        if (strstr(entry->d_name, "__.SYMDEF") != NULL) {
            continue;
        }
#endif
        if (entry->d_name[0] != '.') {
            push_count_chars(&tmpdir_cc, entry->d_name);
            name = flatten_count_chars(&tmpdir_cc, 0);
            if (cmd_data->options.debug) {
                printf("Adding: %s\n", name);
            }
            push_count_chars(cmd_data->obj_files, name);
            pop_count_chars(&tmpdir_cc);
        }
    }

    closedir(dir);
    return 0;
}

static int parse_input_file_name(char *arg, command_t *cmd_data)
{
    const char *ext = strrchr(arg, '.');
    const char *name;
    int pathlen;
    enum lib_type libtype;
    const char *newarg;

    if (!ext) {
        return 0;
    }

    ext++;
    name = jlibtool_basename(arg);

    pathlen = name - arg;

    if ((strcmp(ext, "lo") == 0) ||
	(strcmp(ext, OBJECT_EXT) == 0)) {
        newarg = check_object_exists(cmd_data, arg, ext - arg);
        if (!newarg) {
	    fprintf(stderr, "Can not find suitable object file for %s\n", arg);
            exit(1);
        }
        if (cmd_data->mode != mLink) {
            push_count_chars(cmd_data->arglist, newarg);
        }
        else {
            push_count_chars(cmd_data->obj_files, newarg);
        }
        return 1;
    }

    if (strcmp(ext, "la") == 0) {
        switch (cmd_data->mode) {
        case mLink:
            /* Try the .libs dir first! */
            newarg = check_library_exists(cmd_data, arg, pathlen, 1, &libtype);
            if (!newarg) {
                /* Try the normal dir next. */
                newarg = check_library_exists(cmd_data, arg, pathlen, 0, &libtype);
                if (!newarg) {
		    fprintf(stderr, "Can not find suitable library for %s\n", arg);
                    exit(1);
                }
            }

            /* It is not ok to just add the file: a library may added with:
               1 - -L path library_name. (For *.so in Linux).
               2 - library_name.
             */
#ifdef ADD_MINUS_L
            if (libtype == type_DYNAMIC_LIB) {
                 add_minus_l(cmd_data->shared_opts.dependencies, newarg);
            } else if (cmd_data->output == otLibrary &&
                       libtype == type_STATIC_LIB) {
                explode_static_lib(cmd_data, newarg);
            } else {
                 push_count_chars(cmd_data->shared_opts.dependencies, newarg);
            }
#else
            if (cmd_data->output == otLibrary && libtype == type_STATIC_LIB) {
                explode_static_lib(cmd_data, newarg);
            }
            else {
                push_count_chars(cmd_data->shared_opts.dependencies, newarg);
            }
#endif
            if (libtype == type_DYNAMIC_LIB) {
                if (cmd_data->options.no_install) {
#ifdef RPATH
                    add_rpath_noinstall(cmd_data->shared_opts.dependencies,
                                        arg, pathlen);
#endif
#ifdef DYNAMIC_LINK_NO_INSTALL
                    /*
                     * This doesn't work as Darwin's linker has no way to
                     * override at link-time the search paths for a
                     * non-installed library.
                     */
                    /*
                    add_dylink_noinstall(cmd_data->shared_opts.dependencies,
                                         arg, pathlen, ext - arg);
                    */
#endif
                }
                else {
#ifdef RPATH
                    add_rpath_file(cmd_data->shared_opts.dependencies, arg);
#endif
                }
            }
            break;
        case mInstall:
            /* If we've already recorded a library to install, we're most
             * likely getting the .la file that we want to install as.
             * The problem is that we need to add it as the directory,
             * not the .la file itself.  Otherwise, we'll do odd things.
             */
            if (cmd_data->output == otLibrary) {
                arg[pathlen] = '\0';
                push_count_chars(cmd_data->arglist, arg);
            }
            else {
                cmd_data->output = otLibrary;
                cmd_data->output_name = arg;
                cmd_data->static_name.install = gen_install_name(arg, 0);
                cmd_data->shared_name.install = gen_install_name(arg, 1);
                cmd_data->module_name.install = gen_install_name(arg, 2);
            }
            break;
        default:
            break;
        }
        return 1;
    }

    if (strcmp(ext, "c") == 0) {
        /* If we don't already have an idea what our output name will be. */
        if (cmd_data->basename == NULL) {
            cmd_data->basename = (char *)malloc(strlen(arg) + 4);
            strcpy(cmd_data->basename, arg);
            strcpy(strrchr(cmd_data->basename, '.') + 1, "lo");

            cmd_data->fake_output_name = jlibtool_basename(cmd_data->basename);
        }

	cmd_data->input = inCfile;
    }

    return 0;
}

static int parse_output_file_name(char *arg, command_t *cmd_data)
{
    const char *name;
    const char *ext;
    char *newarg = NULL;
    int pathlen;

    cmd_data->fake_output_name = arg;

    name = jlibtool_basename(arg);
    ext = strrchr(name, '.');

#ifdef EXE_EXT
    if (!ext || strcmp(ext, EXE_EXT) == 0) {
#else
    if (!ext) {
#endif
        cmd_data->basename = arg;
        cmd_data->output = otProgram;
#if defined(_OSD_POSIX)
        cmd_data->options.pic_mode = pic_AVOID;
#endif
        newarg = malloc(strlen(arg) + 4 + sizeof(OBJDIRp));
        strcpy(newarg, arg);
#ifdef EXE_EXT
	if (!ext) {
	  	strcat(newarg, EXE_EXT);
	}
#endif

	add_dotlibs(newarg);
        cmd_data->output_name = newarg;
        return 1;
    }

    ext++;
    pathlen = name - arg;

    if (strcmp(ext, "la") == 0) {
	    if (cmd_data->mode != mLink) {
		    fprintf(stderr, "Inconsistent arguments.  Can only create '.la' when linking\n");
		    exit(1);
	    }

        cmd_data->basename = arg;
	if (cmd_data->options.shared == share_UNSET) {
	  cmd_data->options.shared = share_SHARED;
	}
	if (cmd_data->output == otUnknown) {
	  cmd_data->output = otLibrary;
	}
        cmd_data->static_name.normal = gen_library_name(arg, type_STATIC_LIB);
        cmd_data->shared_name.normal = gen_library_name(arg, type_DYNAMIC_LIB);
        cmd_data->module_name.normal = gen_library_name(arg, type_MODULE_LIB);
        cmd_data->static_name.install = gen_install_name(arg, type_STATIC_LIB);
        cmd_data->shared_name.install = gen_install_name(arg, type_DYNAMIC_LIB);
        cmd_data->module_name.install = gen_install_name(arg, type_MODULE_LIB);

        if (!cmd_data->options.dry_run) {
		const char *base;

		base = jlibtool_basename(cmd_data->static_name.normal);
		if (base == cmd_data->static_name.normal) {
			/* Check first to see if the dir already exists! */
			safe_mkdir(OBJDIR);
		} else {
			char libdir[PATH_MAX];

			memcpy(libdir, cmd_data->static_name.normal,
			       base - cmd_data->static_name.normal);
			libdir[base - cmd_data->static_name.normal] = '\0';
			safe_mkdir(libdir);
		}
        }

#ifdef TRUNCATE_DLL_NAME
        if (shared) {
          arg = truncate_dll_name(arg);
        }
#endif

        cmd_data->output_name = arg;
        return 1;
    }

    if (strcmp(ext, STATIC_LIB_EXT) == 0) {
	    if (cmd_data->mode != mLink) {
		    fprintf(stderr, "Inconsistent arguments.  Can only create '." STATIC_LIB_EXT "' when linking\n");
		    exit(1);
	    }

        cmd_data->basename = arg;
	cmd_data->options.shared = share_STATIC;
	cmd_data->output = otStaticLibraryOnly;
        cmd_data->static_name.normal = gen_library_name(arg, type_STATIC_LIB);
        cmd_data->static_name.install = gen_install_name(arg, type_STATIC_LIB);

        if (!cmd_data->options.dry_run) {
		char *newname;
		char *newext;
		newname = malloc(strlen(cmd_data->static_name.normal) + 1);

		strcpy(newname, cmd_data->static_name.normal);
		newext = strrchr(newname, '/');
		if (!newext) {
			/* Check first to see if the dir already exists! */
			safe_mkdir(OBJDIR);
		} else {
			*newext = '\0';
			safe_mkdir(newname);
		}
		free(newname);
        }

        cmd_data->output_name = arg;
        return 1;
    }

    if (strcmp(ext, DYNAMIC_LIB_EXT) == 0) {
	    if (cmd_data->mode != mLink) {
		    fprintf(stderr, "Inconsistent arguments.  Can only create '." DYNAMIC_LIB_EXT "' when linking\n");
		    exit(1);
	    }

        cmd_data->basename = arg;
	cmd_data->options.shared = share_SHARED;
	cmd_data->output = otDynamicLibraryOnly;
        cmd_data->shared_name.normal = gen_library_name(arg, type_DYNAMIC_LIB);
        cmd_data->module_name.normal = gen_library_name(arg, type_MODULE_LIB);
        cmd_data->shared_name.install = gen_install_name(arg, type_DYNAMIC_LIB);
        cmd_data->module_name.install = gen_install_name(arg, type_MODULE_LIB);

        if (!cmd_data->options.dry_run) {
		char *newname;
		char *newext;
		newname = malloc(strlen(cmd_data->shared_name.normal) + 1);

		strcpy(newname, cmd_data->shared_name.normal);
		newext = strrchr(newname, '/');
		if (!newext) {
			/* Check first to see if the dir already exists! */
			safe_mkdir(OBJDIR);
		} else {
			*newext = '\0';
			safe_mkdir(newname);
		}
		free(newname);
        }

        cmd_data->output_name = arg;
        return 1;
    }

    if (strcmp(ext, "lo") == 0) {
        char *newext;
        cmd_data->basename = arg;
        cmd_data->output = otObject;
        newarg = (char *)malloc(strlen(arg) + 2);
        strcpy(newarg, arg);
        newext = strrchr(newarg, '.') + 1;
        strcpy(newext, OBJECT_EXT);
        cmd_data->output_name = newarg;
        return 1;
    }

    if (strcmp(ext, OBJECT_EXT) == 0) {
        cmd_data->output = otObject;
        cmd_data->basename = arg;
	cmd_data->output_name = arg;
        return 1;
    }

    if (strcmp(ext, DYNAMIC_LIB_EXT) == 0) {
      fprintf(stderr, "Please build libraries with .la target, not ."
	      DYNAMIC_LIB_EXT "\n");
      exit(1);
    }

    if (strcmp(ext, STATIC_LIB_EXT) == 0) {
      fprintf(stderr, "Please build libraries with .la target, not ."
	      STATIC_LIB_EXT "\n");
      exit(1);
    }

    if (cmd_data->output == otUnknown) {
      cmd_data->output = otProgram;
    }
    
    return 0;
}

static void parse_args(int argc, char *argv[], command_t *cmd_data)
{
    int a;
    char *arg, *base;
    int argused;
    int dashc = 0;

    /*
     *  If we're called as something OTHER than jlibtool,
     *  e.g. "program", check if ".libs/program" exists.
     *  If so, assume "--mode=execute program"
     *
     *  This could be fixed to be a little more robust by
     *  checking if argv[0] is a symlink.
     */
    base = jlibtool_basename(argv[0]);
    if (base != argv[0]) {
	    FILE *fp;
	    char buffer[PATH_MAX];

	    strcpy(buffer, argv[0]);
	    add_dotlibs(buffer);

	    fp = fopen(buffer, "r"); /* don't create it */
	    if (fp) {
		    fclose(fp);
		    cmd_data->mode = mExecute;
		    cmd_data->options.silent = 1;
		    push_count_chars(cmd_data->arglist, buffer);
	    }
    }

    /*
     *	We first pass over the command-line arguments looking for
     *  "--mode", etc.  If so, then use the libtool compatibility
     *  method for building the software.  Otherwise, auto-detect it
     *  via "-o" and the extensions.
     */
    base = NULL;
    if (cmd_data->mode == mUnknown) for (a = 1; a < argc; a++) {
        arg = argv[a];

	if (strncmp(arg, "--mode=", 7) == 0) {
	  base = NULL;
	  break;
	}

	if (strncmp(arg, "-o", 2) == 0) {
	  base = argv[++a];
	}
    }

    if (base) {
	    arg = strrchr(base, '.');
	    if (!arg) {
		    cmd_data->mode = mLink;
		    push_count_chars(cmd_data->arglist, LINK_c);
	    }
#ifdef EXE_EXT
	    else if (strcmp(arg, EXE_EXT) == 0) {
		    cmd_data->mode = mLink;
		    push_count_chars(cmd_data->arglist, LINK_c);
	    }
#endif
	    else if (strcmp(arg + 1, DYNAMIC_LIB_EXT) == 0) {
		    cmd_data->mode = mLink;
		    push_count_chars(cmd_data->arglist, LINK_c);
	    }
	    else if (strcmp(arg + 1, STATIC_LIB_EXT) == 0) {
		    cmd_data->mode = mLink;
		    push_count_chars(cmd_data->arglist, LINK_c);
	    }
	    else if (strcmp(arg + 1, "la") == 0) {
		    cmd_data->mode = mLink;
		    push_count_chars(cmd_data->arglist, LINK_c);
	    }
	    else if ((strcmp(arg + 1, "lo") == 0) ||
		     (strcmp(arg + 1, OBJECT_EXT) == 0)) {
		    cmd_data->mode = mCompile;
		    push_count_chars(cmd_data->arglist, CC);
	    }
    }

    for (a = 1; a < argc; a++) {
        arg = argv[a];
        argused = 1;

        if (arg[0] == '-') {
            if (arg[1] == '-') {
                argused = parse_long_opt(arg + 2, cmd_data);
            }
            else {
                argused = parse_short_opt(arg + 1, cmd_data);
            }

            /* We haven't done anything with it yet, try some of the
             * more complicated short opts... */
            if (argused == 0 && a + 1 < argc) {
                if (arg[1] == 'o' && !arg[2]) {
                    arg = argv[++a];
                    argused = parse_output_file_name(arg, cmd_data);

		} else if (arg[1] == 'c' && !arg[2]) {
		    dashc = 1;

                } else if (strcmp(arg+1, "MT") == 0) {
                    if (cmd_data->options.debug) {
                        printf("Adding: %s\n", arg);
                    }
                    push_count_chars(cmd_data->arglist, arg);
                    arg = argv[++a];
                    if (!cmd_data->options.silent) {
                        printf(" %s\n", arg);
                    }
                    push_count_chars(cmd_data->arglist, arg);
                    argused = 1;
                } else if (strcmp(arg+1, "rpath") == 0) {
                    /* Aha, we should try to link both! */
                    cmd_data->install_path = argv[++a];
                    argused = 1;
                } else if (strcmp(arg+1, "release") == 0) {
                    /* Store for later deciphering */
                    cmd_data->version_info = argv[++a];
                    argused = 1;
                } else if (strcmp(arg+1, "version-info") == 0) {
                    /* Store for later deciphering */
                    cmd_data->version_info = argv[++a];
                    argused = 1;
                } else if (strcmp(arg+1, "export-symbols-regex") == 0) {
                    /* Skip the argument. */
                    ++a;
                    argused = 1;
                } else if (strcmp(arg+1, "release") == 0) {
                    /* Skip the argument. */
                    ++a;
                    argused = 1;
                } else if (strcmp(arg+1, "undefined") == 0) {
                    cmd_data->undefined_flag = argv[++a];
                    argused = 1;
                } else if (arg[1] == 'R' && !arg[2]) {
                    /* -R dir Add dir to runtime library search path. */
                    add_runtimedirlib(argv[++a], cmd_data);
                    argused = 1;
                }
            }
        } else {
            argused = parse_input_file_name(arg, cmd_data);
        }

        if (!argused) {
            if (cmd_data->options.debug) {
                printf("Adding: %s\n", arg);
            }
            push_count_chars(cmd_data->arglist, arg);
        }
    }

    if ((cmd_data->output == otObject) && (cmd_data->input == inCfile) &&
	!dashc) {
	    push_count_chars(cmd_data->arglist, "-c");
    }

}

#ifdef GEN_EXPORTS
static void generate_def_file(command_t *cmd_data)
{
    char def_file[1024];
    char implib_file[1024];
    char *ext;
    FILE *hDef;
    char *export_args[1024];
    int num_export_args = 0;
    char *cmd;
    int cmd_size = 0;
    int a;

    if (cmd_data->output_name) {
        strcpy(def_file, cmd_data->output_name);
        strcat(def_file, ".def");
        hDef = fopen(def_file, "w");

        if (hDef != NULL) {
            fprintf(hDef, "LIBRARY '%s' INITINSTANCE\n", nameof(cmd_data->output_name));
            fprintf(hDef, "DATA NONSHARED\n");
            fprintf(hDef, "EXPORTS\n");
            fclose(hDef);

            for (a = 0; a < cmd_data->num_obj_files; a++) {
                cmd_size += strlen(cmd_data->obj_files[a]) + 1;
            }

            cmd_size += strlen(GEN_EXPORTS) + strlen(def_file) + 3;
            cmd = (char *)malloc(cmd_size);
            strcpy(cmd, GEN_EXPORTS);

            for (a=0; a < cmd_data->num_obj_files; a++) {
                strcat(cmd, " ");
                strcat(cmd, cmd_data->obj_files[a] );
            }

            strcat(cmd, ">>");
            strcat(cmd, def_file);
            puts(cmd);
            export_args[num_export_args++] = SHELL_CMD;
            export_args[num_export_args++] = "-c";
            export_args[num_export_args++] = cmd;
            export_args[num_export_args++] = NULL;
            external_spawn(cmd_data, export_args[0], (const char**)export_args);
            cmd_data->arglist[cmd_data->num_args++] = strdup(def_file);

            /* Now make an import library for the dll */
            num_export_args = 0;
            export_args[num_export_args++] = DEF2IMPLIB_CMD;
            export_args[num_export_args++] = "-o";

            strcpy(implib_file, OBJDIRp);
            strcat(implib_file, cmd_data->basename);
            ext = strrchr(implib_file, '.');

            if (ext)
                *ext = 0;

            strcat(implib_file, ".");
            strcat(implib_file, STATIC_LIB_EXT);

            export_args[num_export_args++] = implib_file;
            export_args[num_export_args++] = def_file;
            export_args[num_export_args++] = NULL;
            external_spawn(cmd_data, export_args[0], (const char**)export_args);

        }
    }
}
#endif

#if 0
static const char* expand_path(const char *relpath)
{
    char foo[PATH_MAX], *newpath;

    getcwd(foo, PATH_MAX-1);
    newpath = (char*)malloc(strlen(foo)+strlen(relpath)+2);
    strcpy(newpath, foo);
    strcat(newpath, "/");
    strcat(newpath, relpath);
    return newpath;
}
#endif

static void link_fixup(command_t *c)
{
    /* If we were passed an -rpath directive, we need to build
     * shared objects too.  Otherwise, we should only create static
     * libraries.
     */
    if (!c->install_path && (c->output == otDynamicLibraryOnly ||
        c->output == otModule || c->output == otLibrary)) {
	if (c->options.shared == share_SHARED) {
		c->install_path = LIBDIR;
	}
    }

    if (c->output == otDynamicLibraryOnly ||
        c->output == otModule ||
        c->output == otLibrary) {

        push_count_chars(c->shared_opts.normal, "-o");
        if (c->output == otModule) {
            push_count_chars(c->shared_opts.normal, c->module_name.normal);
        }
        else {
            char *tmp;
            push_count_chars(c->shared_opts.normal, c->shared_name.normal);
#ifdef DYNAMIC_INSTALL_NAME
            push_count_chars(c->shared_opts.normal, DYNAMIC_INSTALL_NAME);

	    if (!c->install_path) c->install_path = LIBDIR;
	    if (!c->shared_name.normal) return;

            tmp = (char*)malloc(PATH_MAX);
            strcpy(tmp, c->install_path);
            strcat(tmp, strrchr(c->shared_name.normal, '/'));
            push_count_chars(c->shared_opts.normal, tmp);
#endif
        }

        append_count_chars(c->shared_opts.normal, c->obj_files);
        append_count_chars(c->shared_opts.normal, c->shared_opts.dependencies);

        if (c->options.export_all) {
#ifdef GEN_EXPORTS
            generate_def_file(c);
#endif
        }
    }

    if (c->output == otLibrary || c->output == otStaticLibraryOnly) {
        push_count_chars(c->static_opts.normal, "-o");
        push_count_chars(c->static_opts.normal, c->output_name);
    }

    if (c->output == otProgram) {
        if (c->output_name) {
            push_count_chars(c->arglist, "-o");
            push_count_chars(c->arglist, c->output_name);
            append_count_chars(c->arglist, c->obj_files);
            append_count_chars(c->arglist, c->shared_opts.dependencies);
            add_dynamic_link_opts(c, c->arglist);
        }
    }
}

static void post_parse_fixup(command_t *cmd_data)
{
    switch (cmd_data->mode)
    {
    case mCompile:
#ifdef PIC_FLAG
	if (cmd_data->options.pic_mode == pic_UNKNOWN) {
		const char *ext;

		/*
		 *	"lo" defaults to "PIC"
		 */
		ext = strrchr(cmd_data->basename, '.');
		if (ext && (strcmp(ext + 1, "lo") == 0)) {
			push_count_chars(cmd_data->arglist, PIC_FLAG);
		}
	} else if (cmd_data->options.pic_mode == pic_PREFER) {
		push_count_chars(cmd_data->arglist, PIC_FLAG);
        }
#endif
        if (cmd_data->output_name) {
            push_count_chars(cmd_data->arglist, "-o");
            push_count_chars(cmd_data->arglist, cmd_data->output_name);
        }
        break;
    case mLink:
        link_fixup(cmd_data);
        break;
    case mInstall:
        if (cmd_data->output == otLibrary) {
            link_fixup(cmd_data);
        }
    default:
        break;
    }

#ifdef USE_OMF
    if (cmd_data->output == otObject ||
        cmd_data->output == otProgram ||
        cmd_data->output == otLibrary ||
        cmd_data->output == otDynamicLibraryOnly) {
        push_count_chars(cmd_data->arglist, "-Zomf");
    }
#endif

    if (cmd_data->options.shared &&
            (cmd_data->output == otObject ||
             cmd_data->output == otLibrary ||
             cmd_data->output == otDynamicLibraryOnly)) {
#ifdef SHARE_SW
        push_count_chars(cmd_data->arglist, SHARE_SW);
#endif
    }
}

static int run_mode(command_t *cmd_data)
{
    int rv;
    count_chars *cctemp;

    cctemp = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cctemp);

    switch (cmd_data->mode)
    {
    case mCompile:
        rv = run_command(cmd_data, cmd_data->arglist);
        if (rv) {
            return rv;
        }
        break;
    case mInstall:
        /* Well, we'll assume it's a file going to a directory... */
        /* For brain-dead install-sh based scripts, we have to repeat
         * the command N-times.  install-sh should die.
         */
        if (!cmd_data->output_name) {
            rv = run_command(cmd_data, cmd_data->arglist);
            if (rv) {
                return rv;
            }
        }
        if (cmd_data->output_name) {
            append_count_chars(cctemp, cmd_data->arglist);
            insert_count_chars(cctemp,
                               cmd_data->output_name,
                               cctemp->num - 1);
            rv = run_command(cmd_data, cctemp);
            if (rv) {
                return rv;
            }
            clear_count_chars(cctemp);
        }
        if (cmd_data->static_name.install) {
            append_count_chars(cctemp, cmd_data->arglist);
            insert_count_chars(cctemp,
                               cmd_data->static_name.install,
                               cctemp->num - 1);
            rv = run_command(cmd_data, cctemp);
            if (rv) {
                return rv;
            }
#if defined(__APPLE__) && defined(RANLIB)
            /* From the Apple libtool(1) manpage on Tiger/10.4:
             * ----
             * With  the way libraries used to be created, errors were possible
             * if the library was modified with ar(1) and  the  table  of
             * contents  was  not updated  by  rerunning ranlib(1).  Thus the
             * link editor, ld, warns when the modification date of a library
             * is more  recent  than  the  creation date  of its table of
             * contents.  Unfortunately, this means that you get the warning
             * even if you only copy the library.
             * ----
             *
             * This means that when we install the static archive, we need to
             * rerun ranlib afterwards.
             */
            const char *lib_args[3], *static_lib_name;
            char *tmp;
            size_t len1, len2;
            len1 = strlen(cmd_data->arglist->vals[cmd_data->arglist->num - 1]);

            static_lib_name = jlibtool_basename(cmd_data->static_name.install);
            len2 = strlen(static_lib_name);

            tmp = malloc(len1 + len2 + 2);

            snprintf(tmp, len1 + len2 + 2, "%s/%s",
                    cmd_data->arglist->vals[cmd_data->arglist->num - 1],
                    static_lib_name);

            lib_args[0] = RANLIB;
            lib_args[1] = tmp;
            lib_args[2] = NULL;
            external_spawn(cmd_data, RANLIB, lib_args);
            free(tmp);
#endif
            clear_count_chars(cctemp);
        }
        if (cmd_data->shared_name.install) {
            append_count_chars(cctemp, cmd_data->arglist);
            insert_count_chars(cctemp,
                               cmd_data->shared_name.install,
                               cctemp->num - 1);
            rv = run_command(cmd_data, cctemp);
            if (rv) {
                return rv;
            }
            clear_count_chars(cctemp);
        }
        if (cmd_data->module_name.install) {
            append_count_chars(cctemp, cmd_data->arglist);
            insert_count_chars(cctemp,
                               cmd_data->module_name.install,
                               cctemp->num - 1);
            rv = run_command(cmd_data, cctemp);
            if (rv) {
                return rv;
            }
            clear_count_chars(cctemp);
        }
        break;
    case mLink:
        if (cmd_data->output == otStaticLibraryOnly ||
            cmd_data->output == otLibrary) {
#ifdef RANLIB
            const char *lib_args[3];
#endif
            /* Removes compiler! */
            cmd_data->program = LIBRARIAN;
            push_count_chars(cmd_data->program_opts, LIBRARIAN_OPTS);
            push_count_chars(cmd_data->program_opts,
                             cmd_data->static_name.normal);

            rv = run_command(cmd_data, cmd_data->obj_files);
            if (rv) {
                return rv;
            }

#ifdef RANLIB
            lib_args[0] = RANLIB;
            lib_args[1] = cmd_data->static_name.normal;
            lib_args[2] = NULL;
            external_spawn(cmd_data, RANLIB, lib_args);
#endif
        }

        if (cmd_data->output == otDynamicLibraryOnly ||
            cmd_data->output == otModule ||
            cmd_data->output == otLibrary) {
            cmd_data->program = NULL;
            clear_count_chars(cmd_data->program_opts);

            append_count_chars(cmd_data->program_opts, cmd_data->arglist);
            if (cmd_data->output == otModule) {
#ifdef MODULE_OPTS
                push_count_chars(cmd_data->program_opts, MODULE_OPTS);
#endif
            } else {
#ifdef SHARED_OPTS
                push_count_chars(cmd_data->program_opts, SHARED_OPTS);
#endif
#ifdef dynamic_link_version_func
                push_count_chars(cmd_data->program_opts,
                             dynamic_link_version_func(cmd_data->version_info));
#endif
            }
            add_dynamic_link_opts(cmd_data, cmd_data->program_opts);

            rv = run_command(cmd_data, cmd_data->shared_opts.normal);
            if (rv) {
                return rv;
            }
        }
        if (cmd_data->output == otProgram) {
            rv = run_command(cmd_data, cmd_data->arglist);
            if (rv) {
                return rv;
            }
        }
        break;
    case mExecute:
    {
	const char *base;
	char *l, libpath[PATH_MAX];

	strcpy(libpath, cmd_data->arglist->vals[0]);
	if (!strstr(cmd_data->arglist->vals[0], OBJDIRp)) {
		add_dotlibs(libpath);
	}

	base = jlibtool_basename(libpath);
	if (base == libpath) {
		l = OBJDIRp;
	} else {
		libpath[base - libpath] = '\0';
		l = libpath;
	}

#ifndef __MINGW32__
	setenv(LD_LIBRARY_PATH_LOCAL, l, 1);
#endif   
	rv = run_command(cmd_data, cmd_data->arglist);
	if (rv) {
	    return rv;
	}
    }
      break;

    default:
        break;
    }

    return 0;
}

static void cleanup_tmp_dir(const char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    char fullname[1024];

    dir = opendir(dirname);

    if (dir == NULL)
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            strcpy(fullname, dirname);
            strcat(fullname, "/");
            strcat(fullname, entry->d_name);
            (void) remove(fullname);
        }
    }

    rmdir(dirname);
}

static void cleanup_tmp_dirs(command_t *cmd_data)
{
    int d;

    for (d = 0; d < cmd_data->tmp_dirs->num; d++) {
        cleanup_tmp_dir(cmd_data->tmp_dirs->vals[d]);
    }
}

static int ensure_fake_uptodate(command_t *cmd_data)
{
    /* FIXME: could do the stat/touch here, but nah... */
    const char *touch_args[3];

    if (cmd_data->mode == mInstall) {
        return 0;
    }
    if (!cmd_data->fake_output_name) {
        return 0;
    }

    /*
     *	Link the "fake" executable to jlibtool.  This lets us run
     *	"./program", without any magic.  jlibtool will then notice
     *	that it's not called "jlibtool", check for "./.libs/program",
     *	and if found, set LD_LIBRARY_PATH and then exec
     *	"./.libs/program"
     */
    if ((cmd_data->mode == mLink) && (cmd_data->output == otProgram) &&
	cmd_data->fake_output_name) {
	    char resolved_path[PATH_MAX];

#ifdef HAS_REALPATH
	    realpath(progname, resolved_path);
#endif      
	
	    unlink(cmd_data->fake_output_name);
#ifndef __MINGW32__      
	    if (symlink(resolved_path, cmd_data->fake_output_name) < 0) {
		    fprintf(stderr, "Error: Can't create %s: %s\n",
			    cmd_data->fake_output_name, strerror(errno));
		    return 1;
	    }
#endif       
	    return 0;
    }

    touch_args[0] = "touch";
    touch_args[1] = cmd_data->fake_output_name;
    touch_args[2] = NULL;
    return external_spawn(cmd_data, "touch", touch_args);
}

/* Store the install path in the *.la file */
static int add_for_runtime(command_t *cmd_data)
{
    if (cmd_data->mode == mInstall) {
        return 0;
    }
    if (cmd_data->output == otDynamicLibraryOnly ||
	cmd_data->output == otLibrary) {
	const char *ext;
        FILE *f;

	ext = cmd_data->fake_output_name;
	if (!ext) ext = cmd_data->output_name;
	ext = strrchr(ext, '.');

	if (ext && (strcmp(ext, ".la") != 0)) {
		const char *ext2, *path;

		path = NULL;
		if (cmd_data->shared_name.normal) {
			ext2 = strrchr(cmd_data->shared_name.normal, '.');
			if (ext2 && (strcmp(ext, ext2) == 0)) {
				path = cmd_data->shared_name.normal;
			}
		}

		if (cmd_data->static_name.normal) {
			ext2 = strrchr(cmd_data->static_name.normal, '.');
			if (ext2 && (strcmp(ext, ext2) == 0)) {
				path = cmd_data->static_name.normal;
			}
		}

		if (path && cmd_data->fake_output_name) {
			unlink(cmd_data->fake_output_name);
#ifndef __MINGW32__         
			if (symlink(path, cmd_data->fake_output_name) < 0) {
				fprintf(stderr, "Error: Can't create %s: %s\n",
					cmd_data->fake_output_name, strerror(errno));
				return -1;
			}
#endif         

			return 0;
		}

		/*
		 *	Else fall back to the old way of doing it.
		 */
	}

	f = fopen(cmd_data->fake_output_name,"w");
        if (f == NULL) {
            return -1;
        }
        fprintf(f,"%s\n", cmd_data->install_path);
        fclose(f);
        return(0);
    } else {
        return(ensure_fake_uptodate(cmd_data));
    }
}

int main(int argc, char *argv[])
{
    int rc;
    command_t cmd_data;

    progname = argv[0];
    memset(&cmd_data, 0, sizeof(cmd_data));

    cmd_data.options.pic_mode = pic_UNKNOWN;

    cmd_data.program_opts = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.program_opts);
    cmd_data.arglist = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.arglist);
    cmd_data.tmp_dirs = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.tmp_dirs);
    cmd_data.obj_files = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.obj_files);
    cmd_data.dep_rpaths = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.dep_rpaths);
    cmd_data.rpaths = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.rpaths);
    cmd_data.static_opts.normal = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.static_opts.normal);
    cmd_data.shared_opts.normal = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.shared_opts.normal);
    cmd_data.shared_opts.dependencies = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.shared_opts.dependencies);

    cmd_data.mode = mUnknown;
    cmd_data.output = otUnknown;

    parse_args(argc, argv, &cmd_data);
    post_parse_fixup(&cmd_data);

    if (cmd_data.mode == mUnknown) {
       usage(1);
    }

    rc = run_mode(&cmd_data);

    if (!rc) {
       add_for_runtime(&cmd_data);
    }

    cleanup_tmp_dirs(&cmd_data);
    return rc;
}
