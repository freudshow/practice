#ifndef SHM_H_
#define SHM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define SHM_KEY_BASE	0x12F45678
#define SEM_KEY_BASE	0x123C56

#define shm1_key	0x13147
#define shm2_key	0x13947
#define shm3_key	0x10147

#define sem1_key	0x13149

#define region_count	10
#define region_size(type)	(region_count*sizeof(type))
#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

union semun {                   /* Used in calls to semctl() */
    int                 val;
    struct semid_ds *   buf;
    unsigned short *    array;
#if defined(__linux__)
    struct seminfo *    __buf;
#endif
};

typedef enum { FALSE, TRUE } Boolean;

typedef struct region_s {
	int longth;
	int width;
	int height;
}region_t;


extern int init_sem_set();
extern int init_region();
extern void write_region(region_t *r, int idx);
extern void read_region(region_t *r, int idx);

#endif
