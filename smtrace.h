/**

-= SQLite Malloc Tracer =-
        Version 1.0.1

(c) 2005 - Marc Noirot

TODO :
- let the user choose the malloc functions at initialization
- do the tracing in another thread to avoid slowing down too much the host application

*/
#ifndef __SMTRACE_H__
#define __SMTRACE_H__

#include <stdlib.h>

#ifdef WIN32
#   ifdef SMTRACE_EXPORTS
#       define SMTRACE_API __declspec(dllexport)
#   else
#       define SMTRACE_API __declspec(dllimport)
#   endif
#else
#   define SMTRACE_API
#endif

/**  
    define REPLACE_STD_MALLOC in your C file to be able to transparently
    use malloc and friends from your app.

    Important : Be careful to have stdlib.h included before smtrace.h,
                else malloc's definition will be kinda messed up.
                Normally this shouldn't be an issue since smtrace.h
                already includes stdlib.h.
*/
#ifdef REPLACE_STD_MALLOC
#   define malloc(x) smt_malloc_loc(x, __FILE__, __LINE__)
#   define realloc(x, y) smt_realloc_loc(x, y, __FILE__, __LINE__)
#   define free(x) smt_free_loc(x, __FILE__, __LINE__)
#   define strdup(x) smt_strdup_loc(x, __FILE__, __LINE__)
#   define calloc(x, y) smt_calloc_loc(x, y, __FILE__, __LINE__)
#endif

/**
    Return codes.
*/
#define SMT_OK                  0
#define SMT_ERR_OPEN_DB         1
#define SMT_ERR_CREATE_TABLES   2
#define SMT_ERR_SESSION_START   3

/**
    Initialize the smtrace library.
    You must call this function before any other one.
    
    @param trace_file the name of the trace database file
    @return SMT_OK if ok, SMT_ERR_* if an error occured
*/
SMTRACE_API int smt_init(const char * trace_file);

/**
    Allocate a chunk of memory from the heap.
    Internally calls malloc from the standard C library.
    
    @param size the number of bytes to allocate
    @return the address of the allocated memory, or NULL if an error occured
*/
SMTRACE_API void * smt_malloc(size_t size);

/**
    Reallocate a chunk of memory from the heap.
    Internally calls realloc from the standard C library.
    
    @param memblock pointer to previously allocated memory block
    @param size new size in bytes
    @return the address of the allocated memory, or NULL if an error occured
*/
SMTRACE_API void * smt_realloc(void * memblock, size_t size);

/**
    Free a chunk of memory previously allocated from the heap.
    Does nothing if the parameter is NULL.
    Internally calls free from the standard C library.
    
    @param memblock pointer to previously allocated memory block
*/
SMTRACE_API void smt_free(void * memblock);

/**
    Duplicate strings.
    Calls malloc and strcpy from the standard C library.

    @param source Null-terminated source string
    @param file usually the C source file from which the function is called (__FILE__)
    @param file usually the C source file's line at which the function is called (__LINE__)
    @return a pointer to the storage location for the copied string 
*/
SMTRACE_API char * smt_strdup(const char * source);

/**
    Allocate an array in memory with elements initialized to 0.
    Calls malloc and memset from the standard C library.
    
    @param num number of elements
    @param size length in bytes of each element
    @return a pointer to the allocated space
*/
SMTRACE_API void * smt_calloc(size_t num, size_t size);

/**
    Allocate a chunk of memory from the heap.
    Internally calls malloc from the standard C library.
    This function takes two more parameters to help debugging.
    
    @param size the number of bytes to allocate
    @param file usually the C source file from which the function is called (__FILE__)
    @param file usually the C source file's line at which the function is called (__LINE__)
    @return the address of the allocated memory, or NULL if an error occured
*/
SMTRACE_API void * smt_malloc_loc(size_t size, const char * file, int line);

/**
    Reallocate a chunk of memory from the heap.
    Internally calls realloc from the standard C library.
    This function takes two more parameters to help debugging.
    
    @param memblock pointer to previously allocated memory block
    @param size new size in bytes
    @param file usually the C source file from which the function is called (__FILE__)
    @param file usually the C source file's line at which the function is called (__LINE__)
    @return the address of the allocated memory, or NULL if an error occured
*/
SMTRACE_API void * smt_realloc_loc(void * memblock, size_t size, const char * file, int line);

/**
    Free a chunk of memory previously allocated from the heap.
    Does nothing if the parameter is NULL.
    Internally calls free from the standard C library.
    This function takes two more parameters to help debugging.
    
    @param memblock pointer to previously allocated memory block
    @param file usually the C source file from which the function is called (__FILE__)
    @param file usually the C source file's line at which the function is called (__LINE__)
*/
SMTRACE_API void smt_free_loc(void * memblock, const char * file, int line);

/**
    Duplicate strings.
    Calls malloc and strcpy from the standard C library.
    This function takes two more parameters to help debugging.

    @param source Null-terminated source string
    @param file usually the C source file from which the function is called (__FILE__)
    @param file usually the C source file's line at which the function is called (__LINE__)
    @return a pointer to the storage location for the copied string 
*/
SMTRACE_API char * smt_strdup_loc(const char * source, const char * file, int line);

/**
    Allocate an array in memory with elements initialized to 0.
    Calls malloc and memset from the standard C library.
    
    @param num number of elements
    @param size length in bytes of each element
    @param file usually the C source file from which the function is called (__FILE__)
    @param file usually the C source file's line at which the function is called (__LINE__)
    @return a pointer to the allocated space
*/
SMTRACE_API void * smt_calloc_loc(size_t num, size_t size, const char * file, int line);

/**
    Close the tracing session started by calling smt_init.
*/
SMTRACE_API void smt_cleanup(void);


#endif //__SMTRACE_H__
