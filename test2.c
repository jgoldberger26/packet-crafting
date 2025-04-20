#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#define PORT 8080  // Change this to the port you want to listen on

// Function to extract the TCP header and filter by destination port
void process_packet(unsigned char *buffer, ssize_t len) {
    struct iphdr *ip_header = (struct iphdr *)buffer;
    if (ip_header->protocol == IPPROTO_TCP) {
        struct tcphdr *tcp_header = (struct tcphdr *)(buffer + (ip_header->ihl * 4));
        if (ntohs(tcp_header->dest) == PORT) {  // Check if destination port matches
            printf("Received packet on port %d from %s:%d\n", PORT,
                   inet_ntoa(*(struct in_addr *)&ip_header->saddr), ntohs(tcp_header->source));
        }
    }
}

int main() {
    int sock;
    char buffer[65536];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    // Create raw socket
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    printf("Listening for TCP packets on port %d...\n", PORT);

    while (1) {
        ssize_t bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
                                          (struct sockaddr *)&src_addr, &addr_len);
        if (bytes_received > 0) {
            process_packet((unsigned char *)buffer, bytes_received);
        }
    }

    close(sock);
    return 0;
}
