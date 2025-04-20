#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    char buffer[65536];  // Buffer to store the packet
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    while (1) {
        ssize_t bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
                                          (struct sockaddr*)&src_addr, &addr_len);
        if (bytes_received < 0) {
            perror("recvfrom failed");
            return 1;
        }

        struct iphdr *ip_header = (struct iphdr*)buffer;
        printf("Received packet from %s\n", inet_ntoa(*(struct in_addr*)&ip_header->saddr));
    }

    close(sock);
    return 0;
}
