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

int main(int argc, char* argv[]) {
	int semid, shmid, bytes, xfrs;
	struct shmseg* shmp;
	union semun dummy;

	semid = semget(SEM_KEY, 2, IPC_CREAT | OBJ_PERMS);
	if (semid == -1)
		fprintf(stderr, "semget");

	if (initSemAvailable(semid, WRITE_SEM) == -1)
		fprintf(stderr, "initSemAvailable");

	if (initSemInUse(semid, READ_SEM) == -1)
		fprintf(stderr, "initSeminUse");

	shmid = shmget(SHM_KEY, sizeof(struct shmseg), IPC_CREAT | OBJ_PERMS);

	if (shmid == -1)
		fprintf(stderr, "shmget");

	shmp = shmat(shmid, NULL, 0);
	if (shmp == (void*)-1)
		fprintf(stderr, "shmat");

	for (xfrs = 0, bytes = 0; ; xfrs++, bytes += shmp->cnt) {
		if (reserveSem(semid, WRITE_SEM) == -1)
			fprintf(stderr, "reserveSem");

		shmp->cnt = read(STDIN_FILENO, shmp->buf, BUF_SIZE);
		if (shmp->cnt == -1)
			fprintf(stderr, "read");

		if (releaseSem(semid, READ_SEM) == -1)
			fprintf(stderr, "releaseSem");

		if (shmp->cnt == 0)
			break;
	}

	if (reserveSem(semid, WRITE_SEM) == -1)
		fprintf(stderr, "reserveSem");

	if (semctl(semid, 0, IPC_RMID, dummy) == -1)
		fprintf(stderr, "semctl");

	if (shmdt(shmp) == -1)
		fprintf(stderr, "shmdt");

	if (shmctl(shmid, IPC_RMID, 0) == -1)
		fprintf(stderr, "shmctl");

	fprintf(stderr, "Sent %d bytes (%d xfrs)\n", bytes, xfrs);
	exit(EXIT_SUCCESS);
}
