/** user_test.c
 *
 */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
int test1() {
    int fd = open("/proc/mytaskinfo", O_RDWR);
    if (fd == -1) {
        printf("Couldn't open file\n");
        return -1;
    }
    char buf[256];

    int r = read(fd, &buf, 256);
    printf("return value: %d\n buf: %.256s\n", r, buf);
    close(fd);
    return 0;
}
int test2() {
    int fd = open("/proc/mytaskinfo", O_RDWR);
    if (fd == -1) {
        printf("Couldn't open file\n");
        return -1;
    }
    char buf;
    int r;

    /* On success, read returns the number of bytes read
    (zero indicates end of file)*/
    while ((r = read(fd, &buf, 1)) > 0) {
        printf("return value: %d\n buf: %c\n", r, buf);
    }
    close(fd);
    return 0;
}

int main() { test2(); }