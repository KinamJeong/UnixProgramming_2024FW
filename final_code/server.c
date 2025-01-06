#include <sys/time.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MSGSIZE 4096

void server(int srvNum, int p1[8][2], int p2[8][2]);
int client(int, int[], int[]);

int main() {
	int pip1[8][2], pip2[8][2];
	int i;

	for (i = 0; i < 8; i++) {
		if (pipe(pip1[i]) == -1)
			perror("pipe call");
		if (pipe(pip2[i]) == -1)
			perror("pipe call");

		switch (fork()) {
		case -1:
			perror("fork call");
		case 0:
			client(i + 1, pip1[i], pip2[i]);
		}
	}


	for (i = 0; i < 2; i++) {
		switch (fork()) {
		case -1:
			perror("fork call");
		case 0:
			server(i, pip1, pip2);
		}
	}


	return(0);
}

void server(int srvNum, int p1[8][2], int p2[8][2])
{
	char buf[8][MSGSIZE], ch;
	fd_set set, master1, master2;
	int i, fd, n, all_read;
	int readFlags[8] = { 0 };
	struct timeval tv1, tv2, tv3;

	for (i = 0; i < 8; i++) {
		close(p1[i][1]);
		close(p2[i][1]);
	}

	FD_ZERO(&master1); FD_ZERO(&master2);

	for (i = 0; i < 8; i++) FD_SET(p1[i][0], &master1);

	while (set = master1, select(p1[7][0] + 1, &set, NULL, NULL, NULL) > 0)
	{
		gettimeofday(&tv1, NULL);

		for (i = 0; i < 8; i++) {
			if (FD_ISSET(p1[i][0], &set)) {
				if ((n = read(p1[i][0], buf[i], MSGSIZE)) > 0) {
					printf("server1 / data receiving... i = %d, read %d bytes\n", i + 1, n);
					readFlags[i] = 1;
				}
			}
		}

		all_read = 1;
		for (i = 0; i < 8; i++) {
			if (readFlags[i] == 0) {
				all_read = 0;
				break;
			}
		}

		if (all_read) {
			gettimeofday(&tv2, NULL);
			fd = open("final_result_1", O_RDWR | O_CREAT | O_TRUNC, 0644);
			for (i = 0; i < 8; i++) {
				n = write(fd, buf[i], sizeof(buf[i]));
				printf("server1 / all data received. i = %d, write %d bytes\n", i + 1, n);
			}
			close(fd);

			gettimeofday(&tv3, NULL);

			long sec1 = tv2.tv_sec - tv1.tv_sec;
			long sec2 = tv3.tv_sec - tv2.tv_sec;
			long micro1 = tv2.tv_usec - tv1.tv_usec;
			long micro2 = tv3.tv_usec - tv2.tv_usec;

			double elapsed1 = sec1 + micro1 * 1e-6;
			printf("server1 - client 통신 시간 : %.6f 초\n", elapsed1);
			double elapsed2 = sec2 + micro2 * 1e-6;
			printf("server1 IO 시간 :            %.6f 초\n", elapsed2);

			break;

		}

	}

	for (i = 0; i < 8; i++) FD_SET(p2[i][0], &master2);

	memset(readFlags, 0, sizeof(readFlags));

	while (set = master2, select(p2[7][0] + 1, &set, NULL, NULL, NULL) > 0)
	{
		gettimeofday(&tv1, NULL);

		for (i = 0; i < 8; i++) {
			if (FD_ISSET(p2[i][0], &set)) {
				if ((n = read(p2[i][0], buf[i], MSGSIZE)) > 0) {
					printf("server2 / data receiving... i = %d, read %d bytes\n", i + 1, n);
					readFlags[i] = 1;
				}
			}
		}

		all_read = 1;
		for (i = 0; i < 8; i++) {
			if (readFlags[i] == 0) {
				all_read = 0;
				break;
			}
		}

		if (all_read) {
			gettimeofday(&tv2, NULL);
			fd = open("final_result_2", O_RDWR | O_CREAT | O_TRUNC, 0644);
			for (i = 0; i < 8; i++) {
				n = write(fd, buf[i], sizeof(buf[i]));
				printf("server2 / all data received. i = %d, write %d bytes\n", i + 1, n);
			}
			close(fd);

			gettimeofday(&tv3, NULL);

			long sec1 = tv2.tv_sec - tv1.tv_sec;
			long sec2 = tv3.tv_sec - tv2.tv_sec;
			long micro1 = tv2.tv_usec - tv1.tv_usec;
			long micro2 = tv3.tv_usec - tv2.tv_usec;

			double elapsed1 = sec1 + micro1 * 1e-6;
			printf("server2 - client 통신 시간 : %.6f 초\n", elapsed1);
			double elapsed2 = sec2 + micro2 * 1e-6;
			printf("server2 IO 시간 :            %.6f 초\n", elapsed2);

			break;
		}

	}

}


int client(int sm, int p1[2], int p2[2]) {
	int n;
	int fd;
	char buffer[1024 * sizeof(int)];
	char filename[256];

	close(p1[0]); close(p2[0]);
	snprintf(filename, sizeof(filename), "SM_%d", sm);
	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		printf("open error\n");
		exit(1);
	}
	n = read(fd, buffer, 1024 * sizeof(int));
	printf("client %d / read bytes from file = %d\n", sm, n);
	write(p1[1], buffer, n);
	n = read(fd, buffer, 1024 * sizeof(int));
	printf("client %d / read bytes from file = %d\n", sm, n);
	write(p2[1], buffer, n);

	close(fd);
	exit(0);
}

