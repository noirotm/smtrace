#include "smtrace.h"
#include "sqlite3.h"
#include <stdlib.h>
#include <string.h>

/* db connection */
static sqlite3* db = NULL;

/* trace session identifier */
static sqlite_int64 session_id = 0;

/* the queries */
static char sync_off_query[] = "PRAGMA synchronous = OFF;";
static char structure_create_query[] = "\
    CREATE TABLE session (\
		id INTEGER PRIMARY KEY AUTOINCREMENT,\
		start_date INTEGER,\
        end_date INTEGER\
	);\
    CREATE TABLE allocation (\
        id INTEGER PRIMARY KEY AUTOINCREMENT,\
        session_id INTEGER,\
        date INTEGER,\
        size INTEGER,\
        address INTEGER,\
        b_reallocation INTEGER,\
        file TEXT,\
        line INTEGER\
    );\
    CREATE TABLE deallocation (\
        id INTEGER PRIMARY KEY AUTOINCREMENT,\
        session_id INTEGER,\
        date INTEGER,\
        address INTEGER,\
        allocation_id INTEGER,\
        b_reallocation INTEGER,\
        file TEXT,\
        line INTEGER\
    );";
static char structure_check_query[] = "SELECT COUNT(name) FROM SQLITE_MASTER";
static char session_init_query[] = "INSERT INTO session VALUES(NULL, strftime('%Y-%m-%d %H:%M:%f','now'), strftime('%Y-%m-%d %H:%M:%f','now'))";
static char session_finalize_query[] = "UPDATE session SET end_date=strftime('%Y-%m-%d %H:%M:%f','now') WHERE id=?";

static char alloc_insert_query[] = "INSERT INTO allocation VALUES(NULL, ?, strftime('%Y-%m-%d %H:%M:%f','now'), ?, ?, ?, ?, ?)";
static char dealloc_insert_query[] = "INSERT INTO deallocation SELECT NULL, ?, strftime('%Y-%m-%d %H:%M:%f','now'), ?, id, ?, ?, ? FROM allocation WHERE address=? ORDER BY date DESC LIMIT 1";

/* the compiled statements */
static struct sqlite3_stmt * alloc_insert_stmt;
static struct sqlite3_stmt * dealloc_insert_stmt;
static struct sqlite3_stmt * session_finalize_stmt;

/* checks whether the database is empty */
int is_database_empty(void)
{
	sqlite3_stmt * stmt;
	if (sqlite3_prepare(db, structure_check_query, -1, &stmt, NULL) == SQLITE_OK) {
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			int count = sqlite3_column_int(stmt, 0);
			sqlite3_finalize(stmt);
			return (count == 0);
		}
		else {
			sqlite3_finalize(stmt);
			return 0;
		}
	}
	else {
		return 0;
	}
}

void trace_alloc(void * memblock, size_t size, const char * file, int line) {
    if (db) {
        sqlite3_bind_int64(alloc_insert_stmt, 1, session_id);
	    sqlite3_bind_int(alloc_insert_stmt, 2, (int)size);
        sqlite3_bind_int(alloc_insert_stmt, 3, (int)memblock);
        sqlite3_bind_int(alloc_insert_stmt, 4, 0);
        sqlite3_bind_text(alloc_insert_stmt, 5, file, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(alloc_insert_stmt, 6, line);
	    sqlite3_step(alloc_insert_stmt);
	    sqlite3_reset(alloc_insert_stmt);
    }
}

void trace_dealloc(void * memblock, const char * file, int line) {
    if (db) {
        sqlite3_bind_int64(dealloc_insert_stmt, 1, session_id);
        sqlite3_bind_int(dealloc_insert_stmt, 2, (int)memblock);
        sqlite3_bind_int(dealloc_insert_stmt, 3, 0);
        sqlite3_bind_text(dealloc_insert_stmt, 4, file, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(dealloc_insert_stmt, 5, line);
        sqlite3_bind_int(dealloc_insert_stmt, 6, (int)memblock);
	    sqlite3_step(dealloc_insert_stmt);
	    sqlite3_reset(dealloc_insert_stmt);
    }
}

void trace_realloc(void * memblock, void * new_memblock, size_t size, const char * file, int line) {
    if (db) {
        sqlite3_bind_int64(dealloc_insert_stmt, 1, session_id);
        sqlite3_bind_int(dealloc_insert_stmt, 2, (int)memblock);
        sqlite3_bind_int(dealloc_insert_stmt, 3, 1);
        sqlite3_bind_text(dealloc_insert_stmt, 4, file, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(dealloc_insert_stmt, 5, line);
        sqlite3_bind_int(dealloc_insert_stmt, 6, (int)memblock);
	    sqlite3_step(dealloc_insert_stmt);
	    sqlite3_reset(dealloc_insert_stmt);

        sqlite3_bind_int64(alloc_insert_stmt, 1, session_id);
	    sqlite3_bind_int(alloc_insert_stmt, 2, (int)size);
        sqlite3_bind_int(alloc_insert_stmt, 3, (int)new_memblock);
        sqlite3_bind_int(alloc_insert_stmt, 4, 1);
        sqlite3_bind_text(alloc_insert_stmt, 5, file, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(alloc_insert_stmt, 6, line);
	    sqlite3_step(alloc_insert_stmt);
	    sqlite3_reset(alloc_insert_stmt);
    }
}

SMTRACE_API int smt_init(const char * trace_file) {
    /* open db */
	if (sqlite3_open(trace_file, &db) != SQLITE_OK)	{
		sqlite3_close(db);
        return SMT_ERR_OPEN_DB;
	}

    /* speed optimization: turn off synchronization */
    sqlite3_exec(db, sync_off_query, NULL, NULL, NULL);
	
	/* build tables if created */
	if (is_database_empty()) {
		if (sqlite3_exec(db, structure_create_query, NULL, NULL, NULL) != SQLITE_OK) {
            return SMT_ERR_CREATE_TABLES;
		}
	}

    /* initialize statments */
    sqlite3_prepare(db, alloc_insert_query, -1, &alloc_insert_stmt, NULL);
    sqlite3_prepare(db, dealloc_insert_query, -1, &dealloc_insert_stmt, NULL);
    sqlite3_prepare(db, session_finalize_query, -1, &session_finalize_stmt, NULL);

    /* start a session */
    if (sqlite3_exec(db, session_init_query, NULL, NULL, NULL) != SQLITE_OK) {
        return SMT_ERR_SESSION_START;
	}

    session_id = sqlite3_last_insert_rowid(db);

    return SMT_OK;
}

SMTRACE_API void * smt_malloc(size_t size) {
    return smt_malloc_loc(size, "", 0);
}

SMTRACE_API void * smt_realloc(void * memblock, size_t size) {
    return smt_realloc_loc(memblock, size, "", 0);
}

SMTRACE_API void smt_free(void * memblock) {
    smt_free_loc(memblock, "", 0);
}

SMTRACE_API char * smt_strdup(const char * source) {
    return smt_strdup_loc(source, "", 0);
}

SMTRACE_API void * smt_calloc(size_t num, size_t size) {
    return smt_calloc_loc(num, size, "", 0);
}

SMTRACE_API void * smt_malloc_loc(size_t size, const char * file, int line) {
    void * memblock = malloc(size);
    trace_alloc(memblock, size, file, line);
    return memblock;
}

SMTRACE_API void * smt_realloc_loc(void * memblock, size_t size, const char * file, int line) {
    void * new_memblock = realloc(memblock, size);
    if (memblock == NULL) {
        /* if memblock is null, then it's a malloc */
        trace_alloc(new_memblock, size, file, line);
    }
    else if (size == 0) {
        /* if size is zero, it's a free */
        trace_dealloc(memblock, file, line);
    }
    else {
        trace_realloc(memblock, new_memblock, size, file, line);
    }
    return new_memblock;
}

SMTRACE_API void smt_free_loc(void * memblock, const char * file, int line) {
    free(memblock);
    trace_dealloc(memblock, file, line);
}

SMTRACE_API char * smt_strdup_loc(const char * source, const char * file, int line) {
    char * new_str = smt_malloc_loc(strlen(source)+1, file, line);
    return strcpy(new_str, source);
}

SMTRACE_API void * smt_calloc_loc(size_t num, size_t size, const char * file, int line) {
    void * memblock = smt_malloc_loc(num * size, file, line);
    return memset(memblock, 0, num * size);
}

SMTRACE_API void smt_cleanup(void) {
    if (!db)
        return;
    /* end the session */
    sqlite3_bind_int64(session_finalize_stmt, 1, session_id);
    sqlite3_step(session_finalize_stmt);
    sqlite3_reset(session_finalize_stmt);

    /* cleanup the statements */
    sqlite3_finalize(alloc_insert_stmt);
    sqlite3_finalize(dealloc_insert_stmt);
    sqlite3_finalize(session_finalize_stmt);

    /* close the database */
    sqlite3_close(db);
    db = NULL;
}
