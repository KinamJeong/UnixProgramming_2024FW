#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>

#define BLOCK_SIZE 16
#define SM_COUNT 8
#define ROWS_PER_FILE 16
#define MSG_COUNT 16
#define MSG_KEY_BASE 1000


struct message {
	long msg_type;
	int data[BLOCK_SIZE];
};

void error_exit(const char* msg) {
	perror(msg);
	exit(1);
}

void close_(int* smfd, int* sm_fd, int* msgq_id) {
	int i;
	for (i = 0; i < SM_COUNT; ++i) {
		close(smfd[i]);
		close(sm_fd[i]);
	}

	for (i = 0; i < SM_COUNT; ++i) {
		if (msgctl(msgq_id[i], IPC_RMID, NULL) == -1) {
			perror("msgctl");
		}
	}
}

int main()
{
	struct timeval start, end;
	long seconds, microseconds;
	double elapsed;

	gettimeofday(&start, NULL);

	int i;
	int smfd[SM_COUNT];
	int sm_fd[SM_COUNT];
	int msgq_id[SM_COUNT];

	for (i = 0; i < SM_COUNT; ++i) {
		key_t key = MSG_KEY_BASE + i;
		if ((msgq_id[i] = msgget(key, IPC_CREAT | 0666)) == -1) {
			error_exit("msgget");
		}
	}

	char filename[20];
	for (i = 0; i < SM_COUNT; ++i) {
		snprintf(filename, sizeof(filename), "SM%d", i + 1);
		if ((smfd[i] = open(filename, O_RDONLY)) == -1) {
			error_exit("open");
		}
	}

	for (i = 0; i < SM_COUNT; ++i) {
		snprintf(filename, sizeof(filename), "SM_%d", i + 1);
		if ((sm_fd[i] = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644)) == -1) {
			error_exit("open");
		}
	}

	int j, k;
	for (i = 0; i < ROWS_PER_FILE; ++i) {
		for (j = 0; j < SM_COUNT; ++j) {
			for (k = 0; k < SM_COUNT; ++k) {

				struct message msg;
				msg.msg_type = 1;

				int offset = (k * BLOCK_SIZE * ROWS_PER_FILE * sizeof(int)) + (i * BLOCK_SIZE * sizeof(int));

				if (lseek(smfd[j], offset, SEEK_SET) == -1)
				{
					close_(smfd, sm_fd, msgq_id);
					error_exit("lseek");
				}

				if (read(smfd[j], msg.data, sizeof(msg.data)) != sizeof(msg.data))
				{

					close_(smfd, sm_fd, msgq_id);
					error_exit("read");
				}

				int SM_index = k;
				if (msgsnd(msgq_id[SM_index], &msg, sizeof(msg.data), 0) == -1)
				{

					close_(smfd, sm_fd, msgq_id);
					error_exit("msgsnd");
				}

				struct message received_msg;
				if (msgrcv(msgq_id[SM_index], &received_msg, sizeof(received_msg.data), 1, 0) == -1)
				{
					close_(smfd, sm_fd, msgq_id);
					error_exit("msgrcv");
				}
				if (write(sm_fd[SM_index], received_msg.data, sizeof(received_msg.data)) != sizeof(received_msg.data))
				{
					close_(smfd, sm_fd, msgq_id);
					error_exit("write");
				}
			}

		}
	}

	close_(smfd, sm_fd, msgq_id);


	gettimeofday(&end, NULL);

	seconds = end.tv_sec - start.tv_sec;
	microseconds = end.tv_usec - start.tv_usec;

	elapsed = seconds + microseconds * 1e-6;

	printf("process execute time : %.6f sec\n", elapsed);


	return 0;

}