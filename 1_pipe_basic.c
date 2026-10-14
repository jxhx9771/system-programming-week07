/*
 * 1_pipe_basic.c — 파이프로 부모가 자식에게 문자열을 보낸다
 *
 * [핵심 개념]
 *   pipe(2) 는 파일 디스크립터 두 개를 준다: fd[0] 은 읽기, fd[1] 은 쓰기.
 *   fork 하면 자식이 이 디스크립터를 물려받으므로 부모-자식이 통신할 수 있다.
 *   각자 쓰지 않는 쪽은 반드시 닫아야 한다 — 쓰기 쪽이 모두 닫혀야 읽기 쪽이 EOF(0)를 본다.
 *   출처: man 2 pipe, man 7 pipe
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 1_pipe_basic 1_pipe_basic.c
 *   ./1_pipe_basic
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int p2c[2];
    int c2p[2];
    char buf[1024];

    if (argc != 2) {
        fprintf(stderr, "사용법: %s \"문자열\"\n", argv[0]);
        return 1;
    }

    if (strlen(argv[1]) >= sizeof(buf)) {
        fprintf(stderr, "문자열이 너무 깁니다.\n");
        return 1;
    }

    if (pipe(p2c) == -1) {
        perror("pipe");
        return 1;
    }

    if (pipe(c2p) == -1) {
        perror("pipe");
        close(p2c[0]);
        close(p2c[1]);
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        close(p2c[0]);
        close(p2c[1]);
        close(c2p[0]);
        close(c2p[1]);
        return 1;
    }

    if (pid == 0) {
        close(p2c[1]);
        close(c2p[0]);

        ssize_t n;
        size_t total = 0;

        while ((n = read(p2c[0], buf + total,
                         sizeof(buf) - 1 - total)) > 0) {
            total += (size_t)n;
            if (total == sizeof(buf) - 1)
                break;
        }

        if (n < 0) {
            perror("read");
            close(p2c[0]);
            close(c2p[1]);
            _exit(1);
        }

        buf[total] = '\0';
        close(p2c[0]);

        printf("[자식] 받은 내용: %s\n", buf);

        for (size_t i = 0; i < total; i++) {
            buf[i] = (char)toupper((unsigned char)buf[i]);
        }

        printf("[자식] 변환한 내용: %s\n", buf);
        fflush(stdout);

        if (write(c2p[1], buf, total) != (ssize_t)total) {
            perror("write");
            close(c2p[1]);
            _exit(1);
        }

        close(c2p[1]);
        _exit(0);
    }

    close(p2c[0]);
    close(c2p[1]);

    printf("[부모] 보낸 내용: %s\n", argv[1]);
    fflush(stdout);

    size_t len = strlen(argv[1]);

    if (write(p2c[1], argv[1], len) != (ssize_t)len) {
        perror("write");
        close(p2c[1]);
        close(c2p[0]);
        waitpid(pid, NULL, 0);
        return 1;
    }

    close(p2c[1]);

    ssize_t n;
    size_t total = 0;

    while ((n = read(c2p[0], buf + total,
                     sizeof(buf) - 1 - total)) > 0) {
        total += (size_t)n;
        if (total == sizeof(buf) - 1)
            break;
    }

    if (n < 0) {
        perror("read");
        close(c2p[0]);
        waitpid(pid, NULL, 0);
        return 1;
    }

    buf[total] = '\0';
    close(c2p[0]);

    printf("[부모] 받은 내용: %s\n", buf);

    if (waitpid(pid, NULL, 0) == -1) {
        perror("waitpid");
        return 1;
    }

    return 0;
}
