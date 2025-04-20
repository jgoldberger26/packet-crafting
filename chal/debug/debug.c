#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

uint32_t PORT;

void process_packet(uint8_t *buffer, ssize_t len, struct sockaddr_in *src_addr, char *str_buf, size_t str_len)
{
    uint8_t *ip_num = (uint8_t *)&src_addr->sin_addr.s_addr;
    sprintf(str_buf, "DEBUG: Data received from %d.%d.%d.%d\n", ip_num[0], ip_num[1], ip_num[2], ip_num[3]);
}

int main()
{
    PORT = atoi(getenv("CHALPORT"));
    if (PORT <= 0) {
        perror("No port defined.\n");
        return 1;
    }


    int sock;
    uint8_t buffer[65536];
    struct sockaddr_in src_addr, srv_addr;
    socklen_t addr_len = sizeof(src_addr);

    char return_str[255];

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
    {
        perror("Socket failed to bind!");
        return 1;
    }

    // printf("Listening for UDP packets on port %d...\n", PORT);

    while (1)
    {
        memset(return_str, 0, sizeof(return_str));
        ssize_t bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
                                          (struct sockaddr *)&src_addr, &addr_len);
        // printf("Packet recieved!\n");

        if (bytes_received > 0)
        {
            process_packet(buffer, bytes_received, &src_addr, return_str, sizeof(return_str));
            sendto(sock, return_str, strlen(return_str), 0, (struct sockaddr *)&src_addr, addr_len);
            // printf("Sending string: %s\n", return_str);
        }
    }

    close(sock);
    return 0;
}
