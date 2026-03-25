#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdarg.h>
#include "headers/includes.h"
#include "headers/util.h"
#include "headers/server.h"

void hexDump(char *desc, void *addr, int len)
{
    unsigned char *pc = (unsigned char*)addr;
    if (desc) printf("%s:\n", desc);
    for (int i = 0; i < len; i++)
    {
        if (i % 16 == 0) printf("  %04x ", i);
        printf(" %02x", pc[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    if (len % 16 != 0) printf("\n");
}

int util_socket_and_bind(struct server *srv)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) return -1;

    struct sockaddr_in bind_addr;
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = 0;
    bind_addr.sin_addr.s_addr = srv->bind_addrs[0];
    
    if (bind(fd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) == -1)
    {
        close(fd);
        return -1;
    }

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    return fd;
}

int util_memsearch(char *buf, int buf_len, char *mem, int mem_len)
{
    for (int i = 0; i <= buf_len - mem_len; i++)
    {
        if (memcmp(buf + i, mem, mem_len) == 0)
            return i;
    }
    return -1;
}

BOOL util_sockprintf(int fd, const char *fmt, ...)
{
    char buffer[BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, BUFFER_SIZE, fmt, args);
    va_end(args);
    
    if (len > 0 && len < BUFFER_SIZE)
    {
        if (send(fd, buffer, len, MSG_NOSIGNAL) == len)
            return TRUE;
    }
    return FALSE;
}

char *util_trim(char *str)
{
    while (isspace(*str)) str++;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = 0;
    return str;
}