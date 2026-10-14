
/*
 * 2_pipe_dup2.c
 * 7주차 과제 2 - dup2를 이용한 명령어 연결
 *
 * 실행 예시: ./2_pipeline ls -l : wc -l
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int fd[2];
    int sep = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ":") == 0) {
            sep = i;
            break;
        }
    }

    if (sep <= 1 || sep >= argc - 1) {
        fprintf(stderr, "사용법: %s 명령1 : 명령2\n", argv[0]);
        return 1;
    }

    argv[sep] = NULL;

    char **cmd1 = &argv[1];
    char **cmd2 = &argv[sep + 1];

    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid1 = fork();

    if (pid1 == -1) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if (pid1 == 0) {
        close(fd[0]);

        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            _exit(1);
        }

        close(fd[1]);

        execvp(cmd1[0], cmd1);

        perror("execvp cmd1");
        _exit(127);
    }

    pid_t pid2 = fork();

    if (pid2 == -1) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        waitpid(pid1, NULL, 0);
        return 1;
    }

    if (pid2 == 0) {
        close(fd[1]);

        if (dup2(fd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            _exit(1);
        }

        close(fd[0]);

        execvp(cmd2[0], cmd2);

        perror("execvp cmd2");
        _exit(127);
    }

    close(fd[0]);
    close(fd[1]);

    int status1, status2;

    if (waitpid(pid1, &status1, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    if (waitpid(pid2, &status2, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    if (!WIFEXITED(status1) || WEXITSTATUS(status1) != 0 ||
        !WIFEXITED(status2) || WEXITSTATUS(status2) != 0) {
        fprintf(stderr, "명령어 실행 중 오류가 발생했습니다.\n");
        return 1;
    }

    return 0;
}