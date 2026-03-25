#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define STDIN   0
#define STDOUT  1
#define STDERR  2

#define FALSE   0
#define TRUE    1
typedef char BOOL;

typedef uint32_t ipv4_t;
typedef uint16_t port_t;

#define INET_ADDR(o1,o2,o3,o4) (htonl((o1 << 24) | (o2 << 16) | (o3 << 8) | (o4 << 0)))

#define ATOMIC_INC(x) __sync_fetch_and_add(x, 1)
#define ATOMIC_DEC(x) __sync_fetch_and_sub(x, 1)
#define ATOMIC_GET(x) __sync_fetch_and_and(x, 0)

#define BUFFER_SIZE 2048