#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define ROWS 128
#define COLS 128
#define BLOCK_SIZE 32
#define NUM_BLOCKS (ROWS / BLOCK_SIZE)  // 4

int main() {
    int matrix[ROWS][COLS];
    int fd;
    int i, j, sm, block_offset;

   
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            matrix[i][j] = i * COLS + j;
        }
    }
  
    fd = open("matrix_file", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        perror("matrix_file ");
        return 1;
    }
    if (write(fd, matrix, sizeof(matrix)) != sizeof(matrix)) {
        perror("matrix_file ");
        close(fd);
        return 1;
    }
    close(fd);

   
    int sm_fds[8];
    for (i = 0; i < 8; i++) {
        char sm_filename[10];
        snprintf(sm_filename, sizeof(sm_filename), "SM%d", i+1);

        sm_fds[i] = open(sm_filename, O_CREAT | O_WRONLY | O_TRUNC | O_APPEND, 0644);
        if (sm_fds[i] == -1) {
            perror("open SM");
            return 1;
        }
    }


    for (sm = 0; sm < 8; sm++) {
        for (block_offset = 0; block_offset < 2; block_offset++) {

           
            int block_num;
            block_num = 8 * block_offset + sm;

            
            int start_row = (block_num / 4) * BLOCK_SIZE;
            int start_col = (block_num % 4) * BLOCK_SIZE;

            int block[BLOCK_SIZE][BLOCK_SIZE];

            for (i = 0; i < BLOCK_SIZE; i++) {
                for (j = 0; j < BLOCK_SIZE; j++) {
                    block[i][j] = matrix[start_row + i][start_col + j];
                }
            }


        
            if (write(sm_fds[sm], block, sizeof(block)) != sizeof(block)) {
                perror("write SM file");
                return 1;
            }
        }
    }


    for (i = 0; i < 8; i++) {
        close(sm_fds[i]);
    }

    return 0;
}
