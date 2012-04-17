SMTrace
=======

This is a simple C library that replaces malloc/free/... and friends
by custom functions for debugging purpose.

These functions use [SQLite3](http://www.sqlite.org/) as a backend to log
every allocation, reallocation or freeing, in order to gather statistics
about memory usage in a program.

Usage
-----

Add

    #define REPLACE_STD_MALLOC
    #include "smtrace.h"

at the top of your C files.

Be wary that the system malloc must be declared before including `smtrace.h`. This is usually done by including
`stdlib.h`.

SMtrace must be initialized as soon as possible in your program by calling `smt_init()`, and cleaned up when
not needed anymore by calling `smt_cleanup()` to ensure
that the sqlite3 database is properly written to disk.

    int main() {
        smt_init("sqlitefile.db");
        // ...
        smt_cleanup();
    }

SQLite3 may allow other programs to access the database while your program is still running, so some kind of
"real-time" monitoring is possible.
It is however meant primarily to analyze memory usage after the program execution has ended.

Limitations
-----------

- This library will slow down your program very significantly. Only use it in a debug situation.
- The database handle is a static object, therefore it is
up to SQLite itself to make sure that the functions can be called without risk from separate threads.
- There are no build files for the moment, the header and C file can be however used as is in your program.
- malloc and friends are substitued via the use of macros, therefore SMTrace will not work in library code
linked to your program. If you need to trace code for which  you do not have access to the source, Valgrind's
[massif](http://valgrind.org/docs/manual/ms-manual.html) tool is recommended instead. It is obviously a very
powerful tool which scope goes way beyond SMtrace.