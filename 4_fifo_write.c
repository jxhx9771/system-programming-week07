
/*
 * 4_fifo_write.c — FIFO 로 메시지를 보낸다  [나중에 실행할 쪽]
 *
 * [핵심 개념]
 *   받는 쪽(3_fifo_read)이 만들어 둔 FIFO 를 열어서 쓰기만 하면 된다.
 *   서로 부모-자식 관계가 아니어도 통신이 된다는 점이 파이프와 다르다.
 *   출처: man 7 fifo
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 4_fifo_write 4_fifo_write.c
 *   ./4_fifo_write 안녕하세요
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO_PATH "/tmp/sysprog_fifo"

int main(int argc, char *argv[])
{
    const char *msg = (argc == 2) ? argv[1] : "기본 메시지";

    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open (받는 쪽을 먼저 실행했는지 확인)");
        return 1;
    }

    if (write(fd, msg, strlen(msg)) != (ssize_t)strlen(msg)) {
        perror("write");
        close(fd);
        return 1;
    }

    printf("보냈습니다: %s\n", msg);

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}
