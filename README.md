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

```C
#define REPLACE_STD_MALLOC
#include "smtrace.h"
```

at the top of your C files.

Be wary that the system malloc must be declared before including `smtrace.h`. This is usually done by including
`stdlib.h`.

SMtrace must be initialized as soon as possible in your program by calling `smt_init()`, and cleaned up when
not needed anymore by calling `smt_cleanup()` to ensure
that the sqlite3 database is properly written to disk.

```C
int main() {
    smt_init("sqlitefile.db");
    // ...
    smt_cleanup();
}
```

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

License
-------

This software is licensed under the terms of the MIT license.

Copyright (C) 2012 Marc Noirot

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions
of the Software.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
