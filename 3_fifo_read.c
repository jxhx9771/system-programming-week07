
/*
 * 3_fifo_read.c — FIFO(이름 있는 파이프)로 메시지를 받는다  [먼저 실행할 쪽]
 *
 * [핵심 개념]
 *   파이프는 부모-자식처럼 관계있는 프로세스끼리만 쓸 수 있다.
 *   FIFO 는 파일 시스템에 이름을 가지므로 아무 관계 없는 두 프로그램도 통신할 수 있다.
 *   open 은 상대가 반대쪽을 열어 줄 때까지 기다린다(블록).
 *   출처: man 3 mkfifo, man 7 fifo
 *
 * [컴파일·실행]  터미널 두 개를 쓴다
 *   gcc -Wall -Wextra -o 3_fifo_read 3_fifo_read.c
 *   ./3_fifo_read          # 먼저 실행 (기다린다)
 *   ./4_fifo_write 안녕     # 다른 터미널에서 실행
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define FIFO_PATH "/tmp/sysprog_fifo"

int main(void)
{
    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        return 1;
    }

    printf("보내는 쪽을 기다립니다... (%s)\n", FIFO_PATH);
    fflush(stdout);

    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("open");
        unlink(FIFO_PATH);
        return 1;
    }

    char buf[128];
    ssize_t n = read(fd, buf, sizeof buf - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("받은 메시지: %s\n", buf);
    } else if (n == -1) {
        perror("read");
    }

    close(fd);
    if (unlink(FIFO_PATH) == -1) {
        perror("unlink");
        return 1;
    }

    printf("FIFO 삭제 완료\n");
    return n == -1 ? 1 : 0;
}
