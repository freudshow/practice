/*
 ============================================================================
 Name        : sqlite.c
 Author      : s_baoshan
 Version     : 0.01
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sqlite3.h>
#include <pthread.h>

#define NO_ERR	0//成功
#define ERR_1	1//失败

#define VACCUM_LOG_FILE	"vacumm.log"

#define DISK_IDLE_SIZE	(4096)
#define DB_LIMIT_SIZE	(41943040L)
#define FILE_LINE	__FILE__,__FUNCTION__,__LINE__
#define DEBUG_TIME_LINE(format, ...)	debugToStderr(FILE_LINE, format, ##__VA_ARGS__)
#define DEBUG_TO_FILE(format, ...)		debugToFile(VACCUM_LOG_FILE, FILE_LINE, format, ##__VA_ARGS__)

typedef  char int8;
typedef  short int16;
typedef  int  int32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef float		float32;

sqlite3 *g_pDB = NULL;

void get_local_time(char* buf, uint32 bufSize)
{
	struct tm timeinfo;
    time_t rawtime;
	
	if ((bufSize < 20) || (NULL == buf))
		return;

    rawtime = time(NULL);
    localtime_r(&rawtime, &timeinfo);

	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",\
	(timeinfo.tm_year+1900), timeinfo.tm_mon+1, timeinfo.tm_mday,\
	timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void debugToFp(FILE *fp, const char* file, const char* func, uint32 line, const char *fmt, ...)
{
	va_list ap;
	char bufTime[20] = { 0 };

	if (NULL == fp)
		return;

	get_local_time(bufTime, sizeof(bufTime));
	fprintf(fp, "\n[pid: %d][%s][%s][%s()][%d]: ", getpid(), bufTime, file, func, line);
	va_start(ap, fmt);
	vfprintf(fp, fmt, ap);
	va_end(ap);
	fprintf(fp, "\n");
}

void debugToStderr(const char* file, const char* func, uint32 line, const char *fmt, ...)
{
	va_list ap;
	char bufTime[20] = { 0 };

	get_local_time(bufTime, sizeof(bufTime));
	fprintf(stderr, "\n[pid: %d][tid: %d][%s][%s][%s()][%d]: ", getpid(), pthread_self(), bufTime, file, func, line);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}

void debugToFile(const char* fname, const char* file, const char* func, uint32 line, const char *fmt,...)
{
	va_list ap;
	FILE *fp = NULL;
	char bufTime[20] = { 0 };

	get_local_time(bufTime, sizeof(bufTime));
	fp = fopen(fname, "a+");//内容存入文件
	if (fp != NULL) {
		fprintf(fp, "\n[pid: %d][tid: %d][%s][%s][%s()][%d]: ", getpid(), pthread_self(), bufTime, file, func, line);
		va_start(ap, fmt);
		vfprintf(fp, fmt, ap);
		va_end(ap);
		fprintf(fp, "\n");
	    fflush(fp);
		fclose(fp);
	}
}

uint8 open_db(char* dbname)
{
	 return (sqlite3_open(dbname, &g_pDB) == SQLITE_OK) ? NO_ERR: ERR_1;
}

uint8 close_db(void)
{
	 return (sqlite3_close(g_pDB) == SQLITE_OK) ? NO_ERR: ERR_1;
}

void exitHandle(int i)
{
	close_db();
	fprintf(stderr, "pid: %d, db closed\n", getpid());
	exit(0);
}

void setSignal(struct sigaction* psa, void (*pfun)(int)) {
    if (psa != NULL) {
        psa->sa_handler = pfun;
        sigemptyset(&psa->sa_mask); //用来将参数 信号集初始化并清空。
        psa->sa_flags = 0;
        sigaction(SIGTERM, psa, NULL);
        sigaction(SIGSYS, psa, NULL);
        sigaction(SIGPWR, psa, NULL);
        sigaction(SIGKILL, psa, NULL);
        sigaction(SIGQUIT, psa, NULL);
        sigaction(SIGILL, psa, NULL);
        sigaction(SIGINT, psa, NULL);
        sigaction(SIGHUP, psa, NULL);
        sigaction(SIGABRT, psa, NULL);
        sigaction(SIGBUS, psa, NULL);
    }
}

int insertImage(sqlite3* db) {
	char* buf = NULL;
	int bufSize = 0;
	FILE *fp = fopen("th.jpg", "rb");
    sqlite3_stmt *pStmt = NULL;
    char *sql = "INSERT INTO Images(f_pid, f_tid, f_oad, Data) VALUES(?, ?, ?, ?)";
    int rc = 0;
    int ret = 0;

    if (fp == NULL) {
        DEBUG_TIME_LINE("Cannot open image file\n");
        ret = 1;
        goto onRet;
    }

    fseek(fp, 0, SEEK_END);
    if (ferror(fp)) {
    	DEBUG_TIME_LINE("fseek() failed\n");
        rc = fclose(fp);
        if (rc == EOF) {
            DEBUG_TIME_LINE("Cannot close file handler\n");
        }
        ret = 1;
        goto onRet;
    }

    int flen = ftell(fp);
    if (flen == -1) {
        perror("error occurred");
        rc = fclose(fp);
        if (rc == EOF) {
            DEBUG_TIME_LINE("Cannot close file handler\n");
        }
        ret = 1;
        goto onRet;
    }

    fseek(fp, 0, SEEK_SET);
    if (ferror(fp)) {
        DEBUG_TIME_LINE("fseek() failed\n");
        rc = fclose(fp);
        if (rc == EOF) {
            DEBUG_TIME_LINE("Cannot close file handler\n");
        }
        ret = 1;
        goto onRet;
    }

    buf = calloc(1, flen+1);
    bufSize = fread(buf, 1, flen, fp);
    if (ferror(fp)) {
        DEBUG_TIME_LINE("fread() failed\n");
        rc = fclose(fp);
        if (rc == EOF)
            DEBUG_TIME_LINE("Cannot close file handler\n");
        ret = 1;
        goto onRet;
    }

    rc = fclose(fp);
    if (rc == EOF) {
        DEBUG_TIME_LINE("Cannot close file handler\n");
        ret = 1;
        goto onRet;
    }

    rc = sqlite3_prepare(db, sql, -1, &pStmt, 0);
    if (rc != SQLITE_OK) {
        DEBUG_TIME_LINE("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        ret = 1;
        goto onRet;
    }

	sqlite3_bind_int(pStmt, 1, getpid());
	sqlite3_bind_int(pStmt, 2, pthread_self());
    sqlite3_bind_blob(pStmt, 3, buf, bufSize, SQLITE_STATIC);
    sqlite3_bind_blob(pStmt, 4, buf, bufSize, SQLITE_STATIC);
    rc = sqlite3_step(pStmt);
    if (rc != SQLITE_DONE) {
        printf("execution failed: %s", sqlite3_errmsg(db));
        ret = 1;
        goto onRet;
    }

onRet:
	if( NULL != buf)
		free(buf);
	if(NULL != pStmt)
		sqlite3_finalize(pStmt);
	return ret;
}

int selectImage(sqlite3 *db) {
	sqlite3_stmt *pStmt = NULL;
	int rc = 0;
	int ret = 0;

    FILE *fp = fopen("th1.jpg", "wb");
    if (fp == NULL) {
        DEBUG_TIME_LINE("Cannot open image file\n");
        ret = 1;
        goto onRet;
    }


    char *sql = "SELECT f_oad, Data FROM Images WHERE Id = 1";


    rc = sqlite3_prepare_v2(db, sql, -1, &pStmt, 0);

    if (rc != SQLITE_OK ) {
        DEBUG_TIME_LINE("Failed to prepare statement\n");
        DEBUG_TIME_LINE("Cannot open database: %s\n", sqlite3_errmsg(db));
        ret = 1;
        goto onRet;
    }

    rc = sqlite3_step(pStmt);
    int bytes = 0;
    if (rc == SQLITE_ROW) {
        bytes = sqlite3_column_bytes(pStmt, 0);
    }

    fwrite(sqlite3_column_blob(pStmt, 0), bytes, 1, fp);

    if (ferror(fp)) {
        DEBUG_TIME_LINE("fwrite() failed\n");
        ret = 1;
        goto onRet;
    }
    int r = fclose(fp);
    if (r == EOF) {
        DEBUG_TIME_LINE("Cannot close file handler\n");
    }

onRet:
	if (NULL != pStmt)
		sqlite3_finalize(pStmt);

    return ret;
}

long get_file_size(char* filename)
{
	long length = 0;
	FILE *fp = NULL;
	fp = fopen(filename, "rb");
	if (fp != NULL) {
		fseek(fp, 0, SEEK_END);
		length = ftell(fp);
	}
	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}
	return length;
}

uint8 db_too_big(char* dbname)
{
	uint8 err = NO_ERR;
	long length = get_file_size(dbname);

	DEBUG_TIME_LINE("db_size: %ld", length);

	return ((length>DB_LIMIT_SIZE) ? ERR_1 : NO_ERR);
}

/*
 * 如果这个仪表的数据表行数超过一定数量
 * 就删除已上传成功的历史数据
 */
uint8 clean_data(char* dbname)
{
	char* pErr;
	uint8 err = NO_ERR;
	char* delete = "delete from images";

	if (db_too_big(dbname) == ERR_1) {/*check if database is too big*/
		DEBUG_TIME_LINE("db is too big, need to vaccum!");
		err = sqlite3_exec(g_pDB, delete, NULL, NULL, &pErr);//删除最早一天的数据
		if(err != SQLITE_OK) {
			return ERR_1;
		}

		err = sqlite3_exec(g_pDB, "vacuum;", NULL, NULL, &pErr);//释放空间
		if(err != SQLITE_OK) {
			return ERR_1;
		}
		DEBUG_TIME_LINE("vaccum done!");
	}

	return err;
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	/*int i;
	for(i=0; i<argc; i++){
		fprintf(stderr ,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	*/
  return 0;
}

int main(int argc, char **argv)
{
	pid_t pid;
	pthread_attr_t thread_attr_1;
	pthread_t thread_1;

	char *zErrMsg = NULL;

	char* dropImage = "drop table if exists \"images\"";
	char* createImage = "create table images(id integer primary key autoincrement, f_pid integer, f_tid integer, f_oad blob, data blob)";

	char *dropTable = "drop table if exists \"t_meter_info\"";
	char *createTable = "create table t_meter_info"\
						"("\
							"f_id integer primary key autoincrement,"\
							"f_meter_type varchar(2),"\
							"f_device_id varchar(4),"\
							"f_meter_address varchar(14),"\
							"f_meter_channel varchar(2),"\
							"f_meter_proto_type varchar(2),"\
							"f_install_pos varchar(50)"\
						")";

  char* insert = "insert into t_meter_info "\
                "("\
					"f_meter_type, "
					"f_device_id, "\
					"f_meter_address, "\
					"f_meter_channel, "\
					"f_meter_proto_type, "\
					"f_install_pos"\
				")"\
				"values "\
				"("\
					"'20', "\
					"1, "\
					"'11110021147872', "\
					"1, "\
					"0, "\
					"'6#管道井'"\
				")";
	char* select = "select * from t_meter_info";

	char* update = "update t_meter_info set f_meter_type='40' where f_id=1";
	int rc;

	struct sigaction sa = {};
	setSignal(&sa, exitHandle);

	DEBUG_TIME_LINE("%s\n", sqlite3_libversion());

	pid = fork();
	if (pid < 0) {
		perror("fork failed");
		exit(1);
	}

	rc = open_db(argv[1]);
		if( rc == ERR_1){
		DEBUG_TIME_LINE("Can't open database: %s\n", sqlite3_errmsg(g_pDB));
		goto onRet;
	}

	if (pid == 0) {
		fprintf(stdout, "This is the child, pid: %d, ppid: %d\n", getpid(), getppid());
	} else {
		fprintf(stdout, "This is the parent, pid: %d, childpid: %d\n", getpid(), pid);

		rc = sqlite3_exec(g_pDB, dropImage, NULL, 0, &zErrMsg);
		if( rc!=SQLITE_OK ){
			DEBUG_TIME_LINE("SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			goto onRet;
		}

		rc = sqlite3_exec(g_pDB, createImage, NULL, 0, &zErrMsg);
		if( rc!=SQLITE_OK ){
			DEBUG_TIME_LINE("SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			goto onRet;
		}

		rc = sqlite3_exec(g_pDB, dropTable, callback, 0, &zErrMsg);
		if( rc!=SQLITE_OK ){
			DEBUG_TIME_LINE("SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			goto onRet;
		}

		rc = sqlite3_exec(g_pDB, createTable, callback, 0, &zErrMsg);
		if( rc!=SQLITE_OK ){
			DEBUG_TIME_LINE("SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			goto onRet;
		}

	}

	while (1) {
		rc = sqlite3_exec(g_pDB, insert, callback, 0, &zErrMsg);
		if( rc!=SQLITE_OK ){
			DEBUG_TIME_LINE("SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			continue;
			//goto onRet;
		}

		rc = sqlite3_exec(g_pDB, select, callback, 0, &zErrMsg);
		if( rc!=SQLITE_OK ){
			DEBUG_TIME_LINE("SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			continue;
			//goto onRet;
		}

		rc = sqlite3_exec(g_pDB, update, callback, 0, &zErrMsg);
		if( rc!=SQLITE_OK ){
			DEBUG_TIME_LINE("SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			continue;
			//goto onRet;
		}

		rc = insertImage(g_pDB);
		if( rc!=0 ){
			DEBUG_TIME_LINE("read image error\n");
			continue;
			//goto onRet;
		}

		rc = selectImage(g_pDB);
		if( rc!=0 ){
			DEBUG_TIME_LINE("read image error\n");
			continue;
			//goto onRet;
		}

		clean_data(argv[1]);
		system("cj stop");
		usleep(1000000);
	}

onRet:
  close_db();
  return 0;
}


int vmain(int argc, char **argv)
{
	unsigned char u8 = 1;

	fprintf(stderr, "u8 d: %d\n", u8);
	fprintf(stderr, "u8 ld: %ld\n", u8);
	DEBUG_TIME_LINE("u8: %d", u8);
	return 0;
}
