SMTrace
=======

This is a simple C library that replaces malloc/free/... and friends
by custom functions for debugging purpose.

These functions use [SQLite3](http://www.sqlite.org/) as a backend to log
every allocation, reallocation or freeing, in order to gather statistics
about memory usage in a program.