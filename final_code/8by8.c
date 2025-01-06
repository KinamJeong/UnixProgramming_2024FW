#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define ROWS 128
#define COLS 128
#define BLOCK_SIZE 16
#define NUM_BLOCKS (ROWS / BLOCK_SIZE)  /*8*/

int main() {
    int matrix[ROWS][COLS];
    int fd;
    int i, j, block_x, block_y;

    /* init 128 x 128 array */
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            matrix[i][j] = i * COLS + j;
        }
    }


	/*  write 128 x 128 array*/
    fd = open("matrix_file", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        perror("matrix_file open");
        return 1;
    }
    if (write(fd, matrix, sizeof(matrix)) != sizeof(matrix)) {
        perror("matrix_file write");
        close(fd);
        return 1;
    }
    close(fd);


	/*  make each SM file and open*/
    int sm_fds[NUM_BLOCKS];
    for (i = 0; i < NUM_BLOCKS; i++) {
        char sm_filename[10];
        snprintf(sm_filename, sizeof(sm_filename), "SM%d", i+1);


        sm_fds[i] = open(sm_filename, O_CREAT | O_WRONLY | O_TRUNC, 0644); /* O_TRUNC*/
        if (sm_fds[i] == -1) {
            perror("open SM file");
            return 1;
        }
    }


	/* write each block to SM*/
    for (block_x = 0; block_x < NUM_BLOCKS; block_x++) {
        for (block_y = 0; block_y < NUM_BLOCKS; block_y++) {
            int block[BLOCK_SIZE][BLOCK_SIZE];


            for (i = 0; i < BLOCK_SIZE; i++) {
                for (j = 0; j < BLOCK_SIZE; j++) {
                    block[i][j] = matrix[block_x * BLOCK_SIZE + i][block_y * BLOCK_SIZE + j];
                }
            }


            if (write(sm_fds[block_y], block, sizeof(block)) != sizeof(block)) {
                perror("write SM file");
                return 1;
            }
        }
    }


    for (i = 0; i < NUM_BLOCKS; i++) {
        close(sm_fds[i]);
    }

    return 0;
}