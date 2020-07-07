#include "shm.h"

static Boolean bsUseSemUndo = FALSE;
static Boolean bsRetryOnEintr = TRUE;

int create_sem(int key, int count)
{
	int semid, shmid;
	region_t *region_p;

	semid = semget(key, count, IPC_CREAT | OBJ_PERMS);
    if (semid == -1) {
        fprintf(stderr, "semget fail: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return semid;
}

int init_one_sem(int semId, int idx, int v)
{
	union semun arg;

    arg.val = v;
    return semctl(semId, idx, SETVAL, arg);
}


int init_sem_set()
{
	int i = 0;
	int semid = create_sem(sem1_key, region_count);

	for(i=0;i<region_count;i++)
		init_one_sem(semid, i, 1);

	printf("sem set init\n");
	return semid;
}

int open_sem(int key)
{
	int semid, shmid;
	region_t *region_p;

	semid = semget(key, 0, 0);
    if (semid == -1) {
        fprintf(stderr, "semget fail: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return semid;
}

int wait_sem(int semId, int idx)
{
    struct sembuf sops;

    sops.sem_num = idx;
    sops.sem_op = -1;
    sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;

    while (semop(semId, &sops, 1) == -1)
        if (errno != EINTR || !bsRetryOnEintr)
            return -1;

    return 0;
}

int post_sem(int semId, int idx)
{
    struct sembuf sops;

    sops.sem_num = idx;
    sops.sem_op = 1;
    sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;

    return semop(semId, &sops, 1);
}

int remove_sem(int semid)
{
	int ret = 0;
	union semun dummy;

    if ((ret = semctl(semid, 0, IPC_RMID, dummy)) == -1) {
        fprintf(stderr, "semctl fail: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return ret;
}

int create_shm(key_t key, size_t size)
{
	int shmid;
	shmid = shmget(key, size, IPC_CREAT | OBJ_PERMS);

    if (shmid == -1) {
        fprintf(stderr, "shmid fail: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("shm created\n");
    return shmid;
}

int open_shm(key_t key)
{
	int shmid;
	shmid = shmget(key, 0, 0);

    if (shmid == -1) {
        fprintf(stderr, "shmid fail: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return shmid;
}

void *attach_shm(int shmid)
{
	void *shmp = shmat(shmid, NULL, 0);
    if (shmp == (void *) -1){
        fprintf(stderr, "shmat fail: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return shmp;
}

int detach_shm(void *shmp)
{
    if (shmdt(shmp) == -1){
        fprintf(stderr, "shmdt fail: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return 0;
}

int remove_shm(int shmid)
{
	int ret = 0;
    if ((ret = shmctl(shmid, IPC_RMID, 0)) == -1) {
        fprintf(stderr, "shmctl fail: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return ret;
}

int init_region()
{
	int semid = open_sem(sem1_key);
	wait_sem(semid, 0);
	int shmid = create_shm(shm1_key, region_size(region_t));
	post_sem(semid, 0);

	return shmid;
}

void write_region(region_t *r, int idx)
{
	region_t *p;
	int semid = open_sem(sem1_key);

	wait_sem(semid, idx);

	int shmid = open_shm(shm1_key);
	p = attach_shm(shmid);
	memcpy(p+idx, r, sizeof(region_t));

	detach_shm(p);

	post_sem(semid, idx);
}

void read_region(region_t *r, int idx)
{
	region_t *p;
	int semid = open_sem(sem1_key);

	wait_sem(semid, idx);

	int shmid = open_shm(shm1_key);
	p = attach_shm(shmid);
	memcpy(r, p+idx, sizeof(region_t));

	detach_shm(p);

	post_sem(semid, idx);
}
