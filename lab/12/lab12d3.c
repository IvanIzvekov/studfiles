#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
typedef enum { FALSE, TRUE } Boolean;
#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define WRITE_SEM 0
#define READ_SEM 1
#define BUF_SIZE 1024
extern Boolean bsUseSemUndo;
extern Boolean bsRetryOnEintr;
struct shmseg {
	int cnt;
	char buf[BUF_SIZE];
};
union semun {
	int val;
	struct semid_ds* buf;
	unsigned short* array;
	struct seminfo* __buf;
};

Boolean bsUseSemUndo = FALSE;
Boolean bsRetryOnEintr = TRUE;

int initSemAvailable(int semId, int semNum) {
	union semun arg;
	arg.val = 1;
	return semctl(semId, semNum, SETVAL, arg);
}

int initSemInUse(int semId, int semNum) {
	union semun arg;
	arg.val = 0;
	return semctl(semId, semNum, SETVAL, arg);
}

int reserveSem(int semId, int semNum) {
	struct sembuf sops;

	sops.sem_num = semNum;
	sops.sem_op = -1;
	sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;

	while (semop(semId, &sops, 1) == -1)
		if (errno != EINTR || !bsRetryOnEintr)
			return -1;

	return 0;
}

int releaseSem(int semId, int semNum) {
	struct sembuf sops;

	sops.sem_num = semNum;
	sops.sem_op = 1;
	sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;

	return semop(semId, &sops, 1);
}
