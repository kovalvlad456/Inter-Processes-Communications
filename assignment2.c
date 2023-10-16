/**
 * Vladyslav Koval
 * CS3305
 * 2023-10-11
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int summation(int start, int end)
{
	int sum = 0;
	if (start < end)
	{
		sum = ((end * (end + 1)) - (start * (start - 1))) / 2;
	}
	return sum;
}

int ith_part_start(int i, int N, int M)
{
	int part_size = N / M;
	int start = i * part_size;
	return start;
}
int ith_part_end(int i, int N, int M)
{
	int part_size = N / M;
	int end = (i < M - 1) ? ((i + 1) * part_size - 1) : N;
	return end;
}
int main(int argc, char **argv)
{
	
	if (argc != 3)
    {
        fprintf(stderr, "Usage: %s N M\n", argv[0]);
        exit(1);
    }

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);

    // Create a pipe
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("Pipe creation has failed");
        exit(1);
    }

    // Create child_1 process
    pid_t child_1_pid = fork();

    if (child_1_pid == -1)
    {
        perror("Fork failed");
        exit(1);
    }
    else if (child_1_pid == 0)
    {
        // Child process (child_1)
        printf("child_1(PID %d): process started from parent(PID %d)\n", getpid(), getppid());
        int total_sum = 0;

        // Fork M child processes
        for (int i = 0; i < M; i++)
        {
            pid_t child_i_pid = fork();
            if (child_i_pid == -1)
            {
                perror("Fork failed");
                exit(1);
            }
            else if (child_i_pid == 0)
            {
                // Child process (child_1.i)
                int start = ith_part_start(i, N, M);
                int end = ith_part_end(i, N, M);
                int partial_sum = summation(start, end);
                printf("child_1.%d(PID %d): fork() successful\n", i + 1, getpid());
                printf("child_1.%d(PID %d): partial sum: [%d - %d] = %d\n", i + 1, getpid(), start, end, partial_sum);

                // Write partial sum to the pipe
                write(pipefd[1], &partial_sum, sizeof(int));
                close(pipefd[1]);
                exit(0);
            }
        }

        // Parent process (child_1)
        for (int i = 0; i < M; i++)
        {
            wait(NULL);
        }

        // Read all partial sums from the pipe and calculate the total sum
        for (int i = 0; i < M; i++)
        {
            int partial_sum;
            read(pipefd[0], &partial_sum, sizeof(int));
            total_sum += partial_sum;
        }
        close(pipefd[0]);

        printf("child_1(PID %d): total sum = %d\n", getpid(), total_sum);
        printf("child_1(PID %d): child_1 completed\n", getpid());
        exit(0);
    }
    else
    {
        // Parent process
        printf("parent(PID %d): process started\n", getpid());
        printf("parent(PID %d): forking child_1\n", getpid());
        printf("parent(PID %d): fork successful for child_1(PID %d)\n", getpid(), child_1_pid);
        printf("parent(PID %d): waiting for child_1(PID %d) to complete\n", getpid(), child_1_pid);

        // Wait for child_1 to complete
        wait(NULL);

        printf("parent(PID %d): parent completed\n", getpid());
    }

    return 0;
}