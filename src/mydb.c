#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>		/* for convenience */
#include <stdlib.h>		/* for convenience */
#include <stddef.h>		/* for offsetof */
#include <string.h>		/* for convenience */
#include <unistd.h>		/* for convenience */
#include <signal.h>		/* for SIG_ERR */
#include <stdarg.h>		/* ISO C variable aruments */
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mydb.h"

#define IDXLEN_SZ	   4	/* index record length (ASCII chars) */
#define SEP         ':'	/* separator char in index record */
#define SPACE       ' '	/* space character */
#define NEWLINE     '\n'	/* newline character */

/*
 * The following definitions are for hash chains and free
 * list chain in the index file.
 */
#define PTR_SZ        7	/* size of ptr field in hash chain */
#define PTR_MAX 9999999	/* max file offset = 10**PTR_SZ - 1 */
#define NHASH_DEF	 479	/* default hash table size */
#define FREE_OFF      0	/* free list offset in index file */
#define HASH_OFF PTR_SZ	/* hash table offset in index file */


/*
 * 一个索引记录
 */
typedef struct {
	off_t nextOffset;
	off_t dataOffset;//数据记录在数据文件中的偏移量
	size_t dataLen;//数据记录的长度
	size_t keyLen;//字符类型的键, 需要包含'\0', 以便字符类函数处理
	char* key;//key是变长数据, 放在最后声明, 以便存储和查找
}dbIdx_s;
typedef dbIdx_s* dbIdx_p;

typedef struct {
	int idxfd; //索引文件的文件描述符
	int datfd; //数据文件的文件描述符
	char *idxbuf; //当前索引记录
	char *datbuf; // 当前数据记录
	char *name; //数据库的文件名
	off_t idxoff; //当前索引记录的偏移量
	size_t idxlen; //当前索引记录的长度
	off_t datoff; //数据记录在数据文件中的偏移量
	size_t datlen; //数据记录的长度
	off_t ptrNext; //下一条索引记录的偏移量
	off_t ptroff; //当前的偏移量
	off_t chainoff; //当前key所在的散列表的表头偏移量
	off_t hashoff; //散列表表头中每条记录的长度
	DBHASH nhash; //散列表的长度, 最好是个素数
	COUNT cnt_delok; /* delete OK */
	COUNT cnt_delerr; /* delete error */
	COUNT cnt_fetchok; /* fetch OK */
	COUNT cnt_fetcherr; /* fetch error */
	COUNT cnt_nextrec; /* nextrec */
	COUNT cnt_stor1; /* store: DB_INSERT, no empty, appended */
	COUNT cnt_stor2; /* store: DB_INSERT, found empty, reused */
	COUNT cnt_stor3; /* store: DB_REPLACE, diff len, appended */
	COUNT cnt_stor4; /* store: DB_REPLACE, same len, overwrote */
	COUNT cnt_storerr; /* store error */
} DB;

int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len) {
	struct flock lock;

	lock.l_type = type; /* F_RDLCK, F_WRLCK, F_UNLCK */
	lock.l_start = offset; /* byte offset, relative to l_whence */
	lock.l_whence = whence; /* SEEK_SET, SEEK_CUR, SEEK_END */
	lock.l_len = len; /* #bytes (0 means to EOF) */

	return (fcntl(fd, cmd, &lock));
}

#define	read_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLK, F_RDLCK, (offset), (whence), (len))
#define	readw_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define	write_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))
#define	writew_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define	un_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))

static void err_doit(int, int, const char *, va_list);

/*
 * Nonfatal error related to a system call.
 * Print a message and return.
 */
void err_ret(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
}

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void err_sys(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}

/*
 * Nonfatal error unrelated to a system call.
 * Error code passed as explict parameter.
 * Print a message and return.
 */
void err_cont(int error, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, error, fmt, ap);
	va_end(ap);
}

/*
 * Fatal error unrelated to a system call.
 * Error code passed as explict parameter.
 * Print a message and terminate.
 */
void err_exit(int error, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, error, fmt, ap);
	va_end(ap);
	exit(1);
}

/*
 * Fatal error related to a system call.
 * Print a message, dump core, and terminate.
 */
void err_dump(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	abort(); /* dump core and terminate */
	exit(1); /* shouldn't get here */
}

/*
 * Nonfatal error unrelated to a system call.
 * Print a message and return.
 */
void err_msg(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
}

/*
 * Fatal error unrelated to a system call.
 * Print a message and terminate.
 */
void err_quit(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
	exit(1);
}

/*
 * Print a message and return to caller.
 * Caller specifies "errnoflag".
 */
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap) {
	char buf[MAXLINE];

	vsnprintf(buf, MAXLINE - 1, fmt, ap);
	if (errnoflag)
		snprintf(buf + strlen(buf), MAXLINE - strlen(buf) - 1, ": %s",
				strerror(error));
	strcat(buf, "\n");
	fflush(stdout); /* in case stdout and stderr are the same */
	fputs(buf, stderr);
	fflush(NULL); /* flushes all stdio output streams */
}

/*
 * Internal functions.
 */
static DB     *_db_alloc(int);
static void    _db_dodelete(DB *);
static int	    _db_find_and_lock(DB *, const char *, int);
static int     _db_findfree(DB *, int, int);
static void    _db_free(DB *);
static DBHASH  _db_hash(DB *, const char *);
static char   *_db_readdat(DB *);
static off_t   _db_readidx(DB *, off_t);
static off_t   _db_readptr(DB *, off_t);
static void    _db_writedat(DB *, const char *, off_t, int);
static void    _db_writeidx(DB *, const char *, off_t, int, off_t);
static void    _db_writeptr(DB *, off_t, off_t);

static DBHASH _db_hash(DB *db, const char *key) {
	DBHASH hval = 0;
	char c;
	int i;

	for (i = 1; (c = *key++) != 0; i++)
		hval += c * i; /* ascii char times its 1-based(means 'i = 1', 0-based means 'i = 0') index */
	return (hval % db->nhash);
}

static DB* _db_alloc(int namelen) {
	DB *db;

	if ((db = calloc(1, sizeof(DB))) == NULL)
		err_dump("_db_alloc: calloc error for DB");
	db->idxfd = db->datfd = -1; /* descriptors */

	/*
	 * 申请数据库文件名.
	 * +5 是因为要加'.idx'和'.dat'等后缀名, 再加1个'\0'.
	 */
	if ((db->name = malloc(namelen + 5)) == NULL)
		err_dump("_db_alloc: malloc error for name");

	if ((db->idxbuf = malloc(IDXLEN_MAX + 2)) == NULL)//+2 for newline and null at end.
		err_dump("_db_alloc: malloc error for index buffer");
	if ((db->datbuf = malloc(DATLEN_MAX + 2)) == NULL)
		err_dump("_db_alloc: malloc error for data buffer");
	return (db);
}

static void _db_free(DB *db) {
	if (db->idxfd >= 0)
		close(db->idxfd);
	if (db->datfd >= 0)
		close(db->datfd);
	if (db->idxbuf != NULL)
		free(db->idxbuf);
	if (db->datbuf != NULL)
		free(db->datbuf);
	if (db->name != NULL)
		free(db->name);
	free(db);
}

/*
 * 在散列表头中读取当前链表的第
 * 1条索引记录在文件中的偏移量
 */
static off_t _db_readptr(DB *db, off_t offset) {
	char asciiptr[PTR_SZ + 1] = { 0 };

	if (lseek(db->idxfd, offset, SEEK_SET) == -1)//先设置偏移量
		err_dump("_db_readptr: lseek error to ptr field");
	if (read(db->idxfd, asciiptr, PTR_SZ) != PTR_SZ)//紧接着读取表头索引记录的偏移量
		err_dump("_db_readptr: read error of ptr field");

	return (atol(asciiptr));
}

/*
 * Read the next index record.  We start at the specified offset
 * in the index file.  We read the index record into db->idxbuf
 * and replace the separators with null bytes.  If all is OK we
 * set db->datoff and db->datlen to the offset and length of the
 * corresponding data record in the data file.
 */
static off_t _db_readidx(DB *db, off_t offset) {
	ssize_t i;
	char *ptr1, *ptr2;
	char asciiptr[PTR_SZ + 1] = {0};//初始化为'\0'
	char asciilen[IDXLEN_SZ + 1] = {0};//初始化为'\0'
	struct iovec iov[2] = { };

	/*
	 * lseek()如果成功, 返回相对于文件起始位置的
	 * 字节数, 将其记录到db->idxoff中.
	 * db_nextrec()调用本函数时, 将offset置0,
	 * 意为从当前位置开始读取.
	 */
	if ((db->idxoff = lseek(db->idxfd, offset,
			offset == 0 ? SEEK_CUR : SEEK_SET)) == -1)
		err_dump("_db_readidx: lseek error");

	iov[0].iov_base = asciiptr;//下一个索引记录的偏移量
	iov[0].iov_len = PTR_SZ;//偏移量长度, 数值长度固定
	iov[1].iov_base = asciilen;//本条索引记录的长度
	iov[1].iov_len = IDXLEN_SZ;//记录长度, 数值长度固定
	if ((i = readv(db->idxfd, &iov[0], 2)) != PTR_SZ + IDXLEN_SZ) {
		if (i == 0 && offset == 0)
			return (-1); /* EOF for db_nextrec */
		err_dump("_db_readidx: readv error of index record");
	}

	db->ptrNext = atol(asciiptr);//下一个索引记录的偏移量
	db->idxlen = atoi(asciilen);//本条索引记录的长度
	if ((db->idxlen) < IDXLEN_MIN || db->idxlen > IDXLEN_MAX)
		err_dump("_db_readidx: invalid length");

	if ((i = read(db->idxfd, db->idxbuf, db->idxlen)) != db->idxlen)
		err_dump("_db_readidx: read error of index record");

	if (db->idxbuf[db->idxlen - 1] != NEWLINE) //完整性检查. sanity check
		err_dump("_db_readidx: missing newline");//sanity - 明智的, 头脑清楚的
	db->idxbuf[db->idxlen - 1] = 0;//终结符'\0'

	/*
	 * 把SEP替换成终结符'\0'
	 */
	if ((ptr1 = strchr(db->idxbuf, SEP)) == NULL)
		err_dump("_db_readidx: missing first separator");
	*ptr1++ = 0; /* replace SEP with null */

	if ((ptr2 = strchr(ptr1, SEP)) == NULL)
		err_dump("_db_readidx: missing second separator");
	*ptr2++ = 0; /* replace SEP with null */

	if (strchr(ptr2, SEP) != NULL)
		err_dump("_db_readidx: too many separators");

	if ((db->datoff = atol(ptr1)) < 0)
		err_dump("_db_readidx: starting offset < 0");
	if ((db->datlen = atol(ptr2)) <= 0 || db->datlen > DATLEN_MAX)
		err_dump("_db_readidx: invalid length");
	return (db->ptrNext);
}

/*
 * 读取一条数据记录
 */
static char *_db_readdat(DB *db) {
	if (lseek(db->datfd, db->datoff, SEEK_SET) == -1)//置偏移量
		err_dump("_db_readdat: lseek error");
	if (read(db->datfd, db->datbuf, db->datlen) != db->datlen)//将数据读取到缓冲区
		err_dump("_db_readdat: read error");
	if (db->datbuf[db->datlen - 1] != NEWLINE)//数据必须以换行符结尾
		err_dump("_db_readdat: missing newline");
	db->datbuf[db->datlen - 1] = 0; /* replace newline with null */
	return (db->datbuf); /* return pointer to data record */
}

/*
 * Find the specified record.  Called by db_delete, db_fetch,
 * and db_store.  Returns with the hash chain locked.
 */
static int _db_find_and_lock(DB *db, const char *key, int writelock) {
	off_t offset, nextoffset;

	db->chainoff = (_db_hash(db, key) * PTR_SZ) + db->hashoff;//表头的偏移量
	db->ptroff = db->chainoff;//哈希表头的偏移量

	printf("%s-%u\n", key, _db_hash(db, key));

	/*
	 * 加锁, 调用者负责释放锁
	 */
	if (writelock) {
		if (writew_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0)
			err_dump("_db_find_and_lock: writew_lock error");
	} else {
		if (readw_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0)
			err_dump("_db_find_and_lock: readw_lock error");
	}

	offset = _db_readptr(db, db->ptroff);//第一个索引记录的偏移量
	while (offset != 0) {
		nextoffset = _db_readidx(db, offset);//下一个索引记录的偏移量
		if (strcmp(db->idxbuf, key) == 0)
			break; //键一致, 找到记录了, 跳出循环

		db->ptroff = offset; /* offset of this (unequal) record */
		offset = nextoffset; /* next one to compare */
	}

	return (offset == 0 ? -1 : 0);
}

/*
 * Delete the current record specified by the DB structure.
 * This function is called by db_delete and db_store, after
 * the record has been located by _db_find_and_lock.
 */
static void _db_dodelete(DB *db) {
	int i;
	char *ptr;
	off_t freeptr, saveptr;

	//数据置' '
	for (ptr = db->datbuf, i = 0; i < db->datlen - 1; i++)
		*ptr++ = SPACE;
	*ptr = 0; //结束符

	//只把键置' ', 遇到结束符就退出了
	ptr = db->idxbuf;
	while (*ptr)
		*ptr++ = SPACE;

	//锁定空闲链表
	if (writew_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
		err_dump("_db_dodelete: writew_lock error");

	//将原有的数据区, 置SPACE
	_db_writedat(db, db->datbuf, db->datoff, SEEK_SET);


	freeptr = _db_readptr(db, FREE_OFF);//读空闲链表第一个记录的位置
	saveptr = db->ptrNext;//保存下一条索引记录的偏移量

	/*
	 * Rewrite the index record.  This also rewrites the length
	 * of the index record, the data offset, and the data length,
	 * none of which has changed, but that's OK.
	 */
	_db_writeidx(db, db->idxbuf, db->idxoff, SEEK_SET, freeptr);

	/*
	 * Write the new free list pointer.
	 */
	_db_writeptr(db, FREE_OFF, db->idxoff);

	/*
	 * Rewrite the chain ptr that pointed to this record being
	 * deleted.  Recall that _db_find_and_lock sets db->ptroff to
	 * point to this chain ptr.  We set this chain ptr to the
	 * contents of the deleted record's chain ptr, saveptr.
	 */
	_db_writeptr(db, db->ptroff, saveptr);
	if (un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
		err_dump("_db_dodelete: un_lock error");
}

/*
 * Write a data record.  Called by _db_dodelete (to write
 * the record with blanks) and db_store.
 */
static void _db_writedat(DB *db, const char *data, off_t offset, int whence) {
	struct iovec iov[2];
	static char newline = NEWLINE;

	/*
	 * If we're appending, we have to lock before doing the lseek
	 * and write to make the two an atomic operation.  If we're
	 * overwriting an existing record, we don't have to lock.
	 */
	if (whence == SEEK_END) /* we're appending, lock entire file */
		if (writew_lock(db->datfd, 0, SEEK_SET, 0) < 0)
			err_dump("_db_writedat: writew_lock error");

	if ((db->datoff = lseek(db->datfd, offset, whence)) == -1)
		err_dump("_db_writedat: lseek error");
	db->datlen = strlen(data) + 1; //文件中用换行符代替'\0'

	iov[0].iov_base = (char *) data;
	iov[0].iov_len = db->datlen - 1;
	iov[1].iov_base = &newline;
	iov[1].iov_len = 1;
	if (writev(db->datfd, &iov[0], 2) != db->datlen)
		err_dump("_db_writedat: writev error of data record");

	if (whence == SEEK_END)
		if (un_lock(db->datfd, 0, SEEK_SET, 0) < 0)
			err_dump("_db_writedat: un_lock error");
}

/*
 * Write an index record.  _db_writedat is called before
 * this function to set the datoff and datlen fields in the
 * DB structure, which we need to write the index record.
 */
static void _db_writeidx(DB *db, const char *key, off_t this_offset, int whence,
		off_t ptrNext) {
	struct iovec iov[2];
	char asciiptrlen[PTR_SZ + IDXLEN_SZ + 1];
	int len;

	db->ptrNext = ptrNext;
	if (ptrNext < 0 || ptrNext > PTR_MAX)
		err_quit("_db_writeidx: invalid ptr: %d", ptrNext);

	sprintf(db->idxbuf, "%s%c%lld%c%ld\n", key, SEP, (long long) db->datoff,
			SEP, (long) db->datlen);
	len = strlen(db->idxbuf);
	if (len < IDXLEN_MIN || len > IDXLEN_MAX)
		err_dump("_db_writeidx: invalid length");
	sprintf(asciiptrlen, "%*lld%*d", PTR_SZ, (long long) ptrNext,
	IDXLEN_SZ, len);

	/*
	 * If we're appending, we have to lock before doing the lseek
	 * and write to make the two an atomic operation.  If we're
	 * overwriting an existing record, we don't have to lock.
	 */
	if (whence == SEEK_END) /* we're appending */
		if (writew_lock(db->idxfd, ((db->nhash+1)*PTR_SZ)+1,
				SEEK_SET, 0) < 0)
			err_dump("_db_writeidx: writew_lock error");

	/*
	 * Position the index file and record the offset.
	 */
	if ((db->idxoff = lseek(db->idxfd, this_offset, whence)) == -1)
		err_dump("_db_writeidx: lseek error");

	iov[0].iov_base = asciiptrlen;
	iov[0].iov_len = PTR_SZ + IDXLEN_SZ;
	iov[1].iov_base = db->idxbuf;
	iov[1].iov_len = len;
	if (writev(db->idxfd, &iov[0], 2) != PTR_SZ + IDXLEN_SZ + len)
		err_dump("_db_writeidx: writev error of index record");

	if (whence == SEEK_END)
		if (un_lock(db->idxfd, ((db->nhash+1)*PTR_SZ)+1,
				SEEK_SET, 0) < 0)
			err_dump("_db_writeidx: un_lock error");
}

/*
 * Write a chain ptr field somewhere in the index file:
 * the free list, the hash table, or in an index record.
 */
static void _db_writeptr(DB *db, off_t offset, off_t ptrval) {
	char asciiptr[PTR_SZ + 1];

	if (ptrval < 0 || ptrval > PTR_MAX)
		err_quit("_db_writeptr: invalid ptr: %d", ptrval);
	sprintf(asciiptr, "%*lld", PTR_SZ, (long long) ptrval);

	if (lseek(db->idxfd, offset, SEEK_SET) == -1)
		err_dump("_db_writeptr: lseek error to ptr field");
	if (write(db->idxfd, asciiptr, PTR_SZ) != PTR_SZ)
		err_dump("_db_writeptr: write error of ptr field");
}

/*
 * Try to find a free index record and accompanying data record
 * of the correct sizes.  We're only called by db_store.
 */
static int _db_findfree(DB *db, int keylen, int datlen) {
	int rc;
	off_t offset, nextoffset, saveoffset;

	/*
	 * Lock the free list.
	 */
	if (writew_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
		err_dump("_db_findfree: writew_lock error");

	/*
	 * Read the free list pointer.
	 */
	saveoffset = FREE_OFF;
	offset = _db_readptr(db, saveoffset);

	while (offset != 0) {
		nextoffset = _db_readidx(db, offset);
		if (strlen(db->idxbuf) == keylen && db->datlen == datlen)
			break; /* found a match */
		saveoffset = offset;
		offset = nextoffset;
	}

	if (offset == 0) {
		rc = -1; /* no match found */
	} else {
		/*
		 * Found a free record with matching sizes.
		 * The index record was read in by _db_readidx above,
		 * which sets db->ptrval.  Also, saveoffset points to
		 * the chain ptr that pointed to this empty record on
		 * the free list.  We set this chain ptr to db->ptrval,
		 * which removes the empty record from the free list.
		 */
		_db_writeptr(db, saveoffset, db->ptrNext);
		rc = 0;

		/*
		 * Notice also that _db_readidx set both db->idxoff
		 * and db->datoff.  This is used by the caller, db_store,
		 * to write the new index record and data record.
		 */
	}

	/*
	 * Unlock the free list.
	 */
	if (un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
		err_dump("_db_findfree: un_lock error");
	return (rc);
}

/*
 * Open or create a database.  Same arguments as open(2).
 */
DBHANDLE db_open(const char *pathname, int oflag, ...)
{
	DB			*db;
	int			len, mode;
	size_t		i;
	/*表头的结构是7*138个ListHead, */
	char		asciiptr[PTR_SZ + 1] = {0};/*plus '1', for ending char '\0'*/
	char		hash[(NHASH_DEF + 1) * PTR_SZ + 2] = {0};/* +2 for newline and null */

	struct stat	statbuff;

	/*
	 * Allocate a DB structure, and the buffers it needs.
	 */
	len = strlen(pathname);
	if ((db = _db_alloc(len)) == NULL)
		err_dump("db_open: _db_alloc error for DB");

	db->nhash   = NHASH_DEF;/* hash table size */
	db->hashoff = HASH_OFF;	/* offset in index file of hash table */
	strcpy(db->name, pathname);
	strcat(db->name, ".idx");

	if (oflag & O_CREAT) {
		va_list ap;

		va_start(ap, oflag);
		mode = va_arg(ap, int);
		va_end(ap);

		/*
		 * Open index file and data file.
		 */
		db->idxfd = open(db->name, oflag, mode);
		strcpy(db->name + len, ".dat");
		db->datfd = open(db->name, oflag, mode);
	} else {
		/*
		 * Open index file and data file.
		 */
		db->idxfd = open(db->name, oflag);
		strcpy(db->name + len, ".dat");
		db->datfd = open(db->name, oflag);
	}

	if (db->idxfd < 0 || db->datfd < 0) {
		_db_free(db);
		return(NULL);
	}

	if ((oflag & (O_CREAT | O_TRUNC)) == (O_CREAT | O_TRUNC)) {
		/*
		 * If the database was created, we have to initialize
		 * it.  Write lock the entire file so that we can stat
		 * it, check its size, and initialize it, atomically.
		 */
		if (writew_lock(db->idxfd, 0, SEEK_SET, 0) < 0)
			err_dump("db_open: writew_lock error");

		if (fstat(db->idxfd, &statbuff) < 0)
			err_sys("db_open: fstat error");

		if (statbuff.st_size == 0) {
			/*
			 * We have to build a list of (NHASH_DEF + 1) chain
			 * ptrs with a value of 0.  The +1 is for the free
			 * list pointer that precedes the hash table.
			 */
			/*
			 * 这里的%*d, 表示占位符, '*'代表这个字符串共占有几个字符,
			 * 字符数由PTR_SZ指定, 具体的字符由"0);"中的0指定
			 */
			sprintf(asciiptr, "%*d", PTR_SZ, 0);/*sprintf()把asciiptr的第一个非可见字符写成'\0'*/
			/*将hash的前138*7个字符, 初始化为'[6个空格]0'*/
			for (i = 0; i < NHASH_DEF + 1; i++)
				strcat(hash, asciiptr);/*strcat自动将第一个非可见字符写为'\0'*/
			strcat(hash, "\n");/*在hash的最后加上换行和字符串结束符'\0'*/
			i = strlen(hash);
			if (write(db->idxfd, hash, i) != i)
				err_dump("db_open: index file init write error");
		}
		if (un_lock(db->idxfd, 0, SEEK_SET, 0) < 0)
			err_dump("db_open: un_lock error");
	}
	db_rewind(db);
	return(db);
}

void db_close(DBHANDLE h) {
	_db_free((DB *) h); /* closes fds, free buffers & struct */
}

/*
 * Fetch a record.  Return a pointer to the null-terminated data.
 */
char *db_fetch(DBHANDLE h, const char *key) {
	DB *db = h;
	char *ptr;

	if (_db_find_and_lock(db, key, 0) < 0) {//如果未找到数据, 错误计数+1
		ptr = NULL;
		db->cnt_fetcherr++;
	} else {
		ptr = _db_readdat(db); /* return pointer to data */
		db->cnt_fetchok++;
	}

	/*
	 * Unlock the hash chain that _db_find_and_lock locked.
	 */
	if (un_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0)
		err_dump("db_fetch: un_lock error");
	return (ptr);
}

/*
 * Delete the specified record.
 */
int db_delete(DBHANDLE h, const char *key) {
	DB *db = h;
	int rc = 0; /* assume record will be found */

	if (_db_find_and_lock(db, key, 1) == 0) {
		_db_dodelete(db);
		db->cnt_delok++;
	} else {
		rc = -1; /* not found */
		db->cnt_delerr++;
	}
	if (un_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0)
		err_dump("db_delete: un_lock error");
	return (rc);
}

/*
 * Store a record in the database.  Return 0 if OK, 1 if record
 * exists and DB_INSERT specified, -1 on error.
 */
int db_store(DBHANDLE h, const char *key, const char *data, int flag) {
	DB *db = h;
	int rc, keylen, datlen;
	off_t ptrval;

	if (flag != DB_INSERT && flag != DB_REPLACE && flag != DB_STORE) {
		errno = EINVAL;
		return (-1);
	}
	keylen = strlen(key);
	datlen = strlen(data) + 1; /* +1 for newline at end */
	if (datlen < DATLEN_MIN || datlen > DATLEN_MAX)
		err_dump("db_store: invalid data length");

	/*
	 * _db_find_and_lock calculates which hash table this new record
	 * goes into (db->chainoff), regardless of whether it already
	 * exists or not. The following calls to _db_writeptr change the
	 * hash table entry for this chain to point to the new record.
	 * The new record is added to the front of the hash chain.
	 */
	if (_db_find_and_lock(db, key, 1) < 0) { /* record not found */
		if (flag == DB_REPLACE) {
			rc = -1;
			db->cnt_storerr++;
			errno = ENOENT; /* error, record does not exist */
			goto doreturn;
		}

		/*
		 * _db_find_and_lock locked the hash chain for us; read
		 * the chain ptr to the first index record on hash chain.
		 */
		ptrval = _db_readptr(db, db->chainoff);

		if (_db_findfree(db, keylen, datlen) < 0) {
			/*
			 * Can't find an empty record big enough. Append the
			 * new record to the ends of the index and data files.
			 */
			_db_writedat(db, data, 0, SEEK_END);
			_db_writeidx(db, key, 0, SEEK_END, ptrval);

			/*
			 * db->idxoff was set by _db_writeidx.  The new
			 * record goes to the front of the hash chain.
			 */
			_db_writeptr(db, db->chainoff, db->idxoff);
			db->cnt_stor1++;
		} else {
			/*
			 * Reuse an empty record. _db_findfree removed it from
			 * the free list and set both db->datoff and db->idxoff.
			 * Reused record goes to the front of the hash chain.
			 */
			_db_writedat(db, data, db->datoff, SEEK_SET);
			_db_writeidx(db, key, db->idxoff, SEEK_SET, ptrval);
			_db_writeptr(db, db->chainoff, db->idxoff);
			db->cnt_stor2++;
		}
	} else { /* record found */
		if (flag == DB_INSERT) {
			rc = 1; /* error, record already in db */
			db->cnt_storerr++;
			goto doreturn;
		}

		/*
		 * We are replacing an existing record.  We know the new
		 * key equals the existing key, but we need to check if
		 * the data records are the same size.
		 */
		if (datlen != db->datlen) {
			_db_dodelete(db); /* delete the existing record */

			/*
			 * Reread the chain ptr in the hash table
			 * (it may change with the deletion).
			 */
			ptrval = _db_readptr(db, db->chainoff);

			/*
			 * Append new index and data records to end of files.
			 */
			_db_writedat(db, data, 0, SEEK_END);
			_db_writeidx(db, key, 0, SEEK_END, ptrval);

			/*
			 * New record goes to the front of the hash chain.
			 */
			_db_writeptr(db, db->chainoff, db->idxoff);
			db->cnt_stor3++;
		} else {
			/*
			 * Same size data, just replace data record.
			 */
			_db_writedat(db, data, db->datoff, SEEK_SET);
			db->cnt_stor4++;
		}
	}
	rc = 0; /* OK */

doreturn: /* unlock hash chain locked by _db_find_and_lock */
	if (un_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0)
		err_dump("db_store: un_lock error");
	return (rc);
}

/*
 * Rewind the index file for db_nextrec.
 * Automatically called by db_open.
 * Must be called before first db_nextrec.
 */
void db_rewind(DBHANDLE h) {
	DB *db = h;
	off_t offset;

	offset = (db->nhash + 1) * PTR_SZ; /* +1 for free list ptr */

	/*
	 * We're just setting the file offset for this process
	 * to the start of the index records; no need to lock.
	 * +1 below for newline at end of hash table.
	 */
	if ((db->idxoff = lseek(db->idxfd, offset + 1, SEEK_SET)) == -1)
		err_dump("db_rewind: lseek error");
}

/*
 * Return the next sequential record.
 * We just step our way through the index file, ignoring deleted
 * records.  db_rewind must be called before this function is
 * called the first time.
 */
char * db_nextrec(DBHANDLE h, char *key) {
	DB *db = h;
	char c;
	char *ptr;

	/*
	 * We read lock the free list so that we don't read
	 * a record in the middle of its being deleted.
	 */
	if (readw_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
		err_dump("db_nextrec: readw_lock error");

	do {
		/*
		 * Read next sequential index record.
		 */
		if (_db_readidx(db, 0) < 0) {
			ptr = NULL; /* end of index file, EOF */
			goto doreturn;
		}

		/*
		 * Check if key is all blank (empty record).
		 */
		ptr = db->idxbuf;
		while ((c = *ptr++) != 0 && c == SPACE)
			; /* skip until null byte or nonblank */
	} while (c == 0); /* loop until a nonblank key is found */

	if (key != NULL)
		strcpy(key, db->idxbuf); /* return key */
	ptr = _db_readdat(db); /* return pointer to data buffer */
	db->cnt_nextrec++;

	doreturn: if (un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
		err_dump("db_nextrec: un_lock error");
	return (ptr);
}

void readData(FILE* fp, DBHANDLE db) {
	char line[1024] = { 0 };
	char* key;
	char* value;
	int len = sizeof(line);

	while (!feof(fp)) {
		if (fgets(line, len, fp) != NULL) {
			key = strtok(line, ",");
			value = strtok(NULL, ",");
			if (db_store(db, key, value, DB_INSERT) != 0)
				err_quit("db_store error for %s", key);
		}
	}
}

int main(int argc, char* argv[]) {
	FILE *fp;
	DBHANDLE db;

	if(argc != 2) {
		err_quit("usage: program csvfile");
	}

	if ((db = db_open("db4", O_RDWR | O_CREAT | O_TRUNC, FILE_MODE)) == NULL)
		err_sys("db_open error");

	fp = fopen(argv[1], "r");
	if (fp == NULL)
		err_sys("file error");
	readData(fp, db);
	fclose(fp);

	db_close(db);
	exit(0);
}
