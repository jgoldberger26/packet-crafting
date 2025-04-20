#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define PORT 8080 // TODO Change to get this from an environment variable
#define ADMIN_IP 0xdeadbeef
#define FLAG "RPISEC{dummy_flag}"

#define ACTION_PING 0
#define ACTION_GET_FLAG 1

// Unix Time: 1740081504
// Max Int:   4294967295

struct app_data
{
    uint32_t time;
    uint8_t action;
};

void process_packet(uint8_t *buffer, ssize_t len, struct sockaddr_in *src_addr, char *str_buf, size_t str_len)
{
    struct app_data *dat = (struct app_data *)buffer;
    uint8_t action = dat->action;

    

    strcpy(str_buf, "Invalid action!\n");
    return;
}

int main()
{
    int sock;
    uint8_t buffer[65536];
    struct sockaddr_in src_addr, srv_addr;
    socklen_t addr_len = sizeof(src_addr);

    char return_str[255];

    // Create UDP socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

    printf("Listening for TCP packets on port %d...\n", PORT);

    while (1)
    {
        memset(return_str, 0, sizeof(return_str));
        ssize_t bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
                                          (struct sockaddr *)&src_addr, &addr_len);
        printf("Packet recieved!\n");
        if (bytes_received > 0)
        {
            process_packet(buffer, bytes_received, &src_addr, return_str, sizeof(return_str));
            sendto(sock, return_str, strlen(return_str), 0, (struct sockaddr *)&src_addr, addr_len);
            printf("Sending string: %s\n", return_str);
        }
    }

    close(sock);
    return 0;
}
