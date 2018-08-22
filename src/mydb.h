#ifndef _MY_DB_H
#define _MY_DB_H

#define	MAXLINE	4096			/* max line length */

/*
 * Default file access permissions for new files.
 */
#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*
 * Default permissions for new directories.
 */
#define	DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)


/*
 * Flags for db_store().
 */
#define DB_INSERT	   1	/* insert new record only */
#define DB_REPLACE	   2	/* replace existing record */
#define DB_STORE	   3	/* replace or insert */

/*
 * Implementation limits.
 */
#define IDXLEN_MIN	   6	/* key, sep, start, sep, length, \n */
#define IDXLEN_MAX	1024	/* arbitrary */
#define DATLEN_MIN	   2	/* data byte, newline */
#define DATLEN_MAX	1024	/* arbitrary */

typedef unsigned long DBHASH; /* hash values */
typedef unsigned long COUNT; /* unsigned counter */

typedef	void *	DBHANDLE;


DBHANDLE  db_open(const char *, int, ...);
void      db_close(DBHANDLE);
char     *db_fetch(DBHANDLE, const char *);
int       db_store(DBHANDLE, const char *, const char *, int);
int       db_delete(DBHANDLE, const char *);
void      db_rewind(DBHANDLE);
char     *db_nextrec(DBHANDLE, char *);

#endif//_MY_DB_H
