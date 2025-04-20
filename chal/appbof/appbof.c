// gcc -g -O0 -fno-stack-protector appbof.c

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
#define ADMIN_IP 0xdeadbeef
#define FLAG "RPISEC{dummy_flag}"

#define ACTION_PING 0
#define ACTION_GET_FLAG 1

struct app_data
{
    uint8_t action;
    uint8_t slen;
    char sdata[232];
};

void process_packet(uint8_t *buffer, char *str_buf, size_t str_len)
{
    struct app_data *dat = (struct app_data *)buffer;
    char sdata[232];
    uint8_t action;

    action = dat->action;
    if (action == ACTION_GET_FLAG)
    {
        strncpy(str_buf, "Flag functionality is permanantly disabled.\n", str_len);
        return;
    }

    memcpy(sdata, dat->sdata, dat->slen);
    switch (action)
    {
    case ACTION_PING:
        strncpy(str_buf, sdata, ((struct app_data*)sdata)->slen);
        str_buf[str_len - 1] = '\0';
        return;
    case ACTION_GET_FLAG:
        sprintf(str_buf, "Welcome, admin! Flag: %s\n", FLAG);
        return;
    default:
        sprintf(str_buf, "%d is not a valid action!\n", action);
        return;
    }
}

// Returns the dst port of the UDP packet, and puts the offset and length of the data into data_offset and data_len
// If processing fails, return -1
int get_packet_data(uint8_t *buffer, uint8_t *data_offset, uint8_t *data_len, uint16_t *connection_port)
{
    // We can always assume buffer is an IPv4 UDP packet because of the socket binding
    uint8_t ihl = buffer[0] & 0x0F;
    if (ihl != 5)
    {
        return -1;
    }

    uint16_t dport = (uint16_t)(buffer[22] << 8) | buffer[23];
    uint16_t sport = (uint16_t)(buffer[20] << 8) | buffer[21];
    *connection_port = ntohs(sport);

    uint16_t udplen = (uint16_t)(buffer[24] << 8) | buffer[25];
    if (udplen < 255)
        *data_len = (uint8_t)udplen;
    else
        *data_len = 255;

    // We only accept 20 byte IP headers and UDP headers are always 8 bytes
    *data_offset = 28;

    return dport;
}

int main()
{

    PORT = atoi(getenv("CHALPORT"));
    if (PORT <= 0) {
        perror("No port defined.\n");
        return 1;
    }

    int raw_sock;
    int udp_sock;
    uint8_t buffer[65536];
    struct sockaddr_in src_addr, srv_addr;
    socklen_t addr_len = sizeof(src_addr);
    uint8_t data_offset = 0;
    uint8_t data_len = 0;
    uint16_t connection_port;

    char return_str[255];

    // Create UDP socket
    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (raw_sock < 0 || udp_sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(PORT);

    if (bind(raw_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
    {
        perror("Socket failed to bind!");
        return 1;
    }
    if (bind(udp_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0)
    {
        perror("Socket failed to bind!");
        return 1;
    }

    // printf("Listening for UDP packets on port %d...\n", PORT);

    while (1)
    {
        memset(return_str, 0, sizeof(return_str));
        ssize_t bytes_received = recvfrom(raw_sock, buffer, sizeof(buffer), 0,
                                          (struct sockaddr *)&src_addr, &addr_len);

        if (bytes_received <= 0)
            continue;

        int port = get_packet_data(buffer, &data_offset, &data_len, &connection_port);
        if (port != PORT)
            continue;
        // printf("Packet recieved on port %d with length %d!\n", port, data_len);

        src_addr.sin_port = connection_port;

        process_packet(buffer + data_offset, return_str, sizeof(return_str));
        sendto(udp_sock, return_str, strlen(return_str), 0, (struct sockaddr *)&src_addr, addr_len);
        // printf("Sending string: %s\n", return_str);
    }

    close(raw_sock);
    close(udp_sock);
    return 0;
}
