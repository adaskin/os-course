/** user_test.c
 *
 */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
char *fname = "/proc/mytaskinfo";

const char *states[9] = {
    "R (running)",      /* 0x00 */
    "S (sleeping)",     /* 0x01 */
    "D (disk sleep)",   /* 0x02 */
    "T (stopped)",      /* 0x04 */
    "t (tracing stop)", /* 0x08 */
    "X (dead)",         /* 0x10 */
    "Z (zombie)",       /* 0x20 */
    "P (parked)",       /* 0x40 */
    "I (idle)",         /* 0x80 */
};
int test1() {
    printf(
        "\n\ntest1............................testing read/write\n");
    printf("openning file...\n");
    int fd = open(fname, O_RDWR);
    if (fd == -1) {
        printf("Couldn't open file\n");
        return -1;
    }
    char buf[1024];
    printf("reading 1024 char WITHOUT initial write...\n");
    int r = read(fd, &buf, 1024);
    printf("return value: %d\n buf: %.1024s\n", r, buf);
    close(fd);
    return 0;
}
int test2() {
    printf(
        "\n\ntest2............................testing read/write\n");
    printf("openning file...\n");
    int fd = open(fname, O_RDWR);
    if (fd == -1) {
        printf("Couldn't open file\n");
        return -1;
    }
    printf("file opened...\n");
    char buf;
    int r;
    printf(
        "reading char by char till EOF WITHOUT initial write...\n");
    /* On success, read returns the number of bytes read
    (zero indicates end of file)*/
    while ((r = read(fd, &buf, 1)) > 0) {
        printf("return value: %d\n buf: %c\n", r, buf);
    }
    printf("Done! closing file...\n");
    close(fd);
    return 0;
}
int test3() {
    printf(
        "\n\ntest3............................testing read/write\n");
    printf("openning file...\n");
    int fd = open(fname, O_RDWR);
    if (fd == -1) {
        printf("Couldn't open file\n");
        return -1;
    }
    printf("file opened...\n");
    char buf[1024];
    int r;
    printf(
        "...........reading 1024-char at a time till EOF WITHOUT "
        "initial "
        "write...\n");
    /* On success, read returns the number of bytes read
    (zero indicates end of file)*/
    while ((r = read(fd, &buf, 1024)) > 0) {
        printf("return value: %d\n buf: %.1024s\n", r, buf);
    }
    printf(".........Done! closing file...\n");
    close(fd);
    return 0;
}
int test4() {
    printf(
        "\n\ntest4............................testing read/write\n"
        "with closing and openning");

    for (int i = 0; i < 9; i++) {
        printf("openning file...\n");
        int fd = open(fname, O_RDWR);
        if (fd == -1) {
            printf("Couldn't open file\n");
            return -1;
        }
        printf("file opened...\n");
        printf("writing %c  to the file...\n", states[i][0]);
        write(fd, &states[i][0], 1);
        printf("write done!..\n");
        char buf[1024];
        int r;
        printf(
            "...........reading 1024 at a time till EOF WITH initial "
            "write...\n");
        /* On success, read returns the number of bytes read
        (zero indicates end of file)*/
        while ((r = read(fd, &buf, 1024)) > 0) {
            printf("return value: %d\n buf: %.1024s\n", r, buf);
        }
        printf(".........Done! closing file...\n");
        close(fd);
    };
    return 0;
}
int test5() {
    printf(
        "\n\ntest5............................testing write/read\n"
        "without closing the file");
    printf("openning file...\n");
    int fd = open(fname, O_RDWR);
    if (fd == -1) {
        printf("Couldn't open file\n");
        return -1;
    }
    printf("file opened...\n");
    for (int i = 0; i < 9; i++) {
        printf("writing %c  to the file...\n", states[i][0]);
        write(fd, &states[i][0], 1);
        printf("write done!..\n");
        char buf[1024];
        int r;
        printf(
            "...........reading 1024 at a time till EOF WITH initial "
            "write...\n");
        /* On success, read returns the number of bytes read
        (zero indicates end of file)*/
        while ((r = read(fd, &buf, 1024)) > 0) {
            printf("return value: %d\n buf: %.1024s\n", r, buf);
        }
    }
    printf(".........Done! closing file...\n");
    close(fd);
    return 0;
}
int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
}
