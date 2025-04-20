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
    char sdata[255];
};

void process_packet(uint8_t *buffer, char *str_buf, size_t str_len, bool is_evil)
{
    struct app_data *dat = (struct app_data *)buffer;

    uint8_t action = dat->action;
    char *sdata = dat->sdata;
    size_t slen = strlen(sdata);

    if (slen >= 255)
    {
        slen = 254; // No overreads!
    }

    if (!is_evil) {
        strncpy(str_buf, "Get out of here with your goody-two-shoes packets. I only accept EVIL packets! Mwahahaha!\n", str_len);
        return;
    }

    switch (action)
    {
    case ACTION_PING:
        strncpy(str_buf, sdata, str_len);
        str_buf[slen] = '\0';
        // printf("Mwahahaha! Evil Pong!\n"); // This doesn't go back to the user
        return;
    case ACTION_GET_FLAG:
        strncpy(str_buf, "Mwahahaha! I see you're just as evil as I am! Flag: ", str_len);
        strcat(str_buf, FLAG);
        strcat(str_buf, "\n");
        return;
    default:
        strncpy(str_buf, "You fool, that's not a valid action!\n\0", str_len);
        return;
    }
}

// Returns the dst port of the UDP packet, and puts the offset and length of the data into data_offset and data_len
// If processing fails, return -1
int get_packet_data(uint8_t *buffer, uint8_t *data_offset, uint8_t *data_len, bool *is_evil, uint16_t *connection_port)
{
    // We can always assume buffer is an IPv4 UDP packet because of the socket binding
    uint8_t ihl = buffer[0] & 0x0F;
    if (ihl != 5)
    {
        return -1;
    }

    uint8_t flags = (buffer[6] & 0xE0) >> 5;
    if (flags & 0x04)
        *is_evil = true;
    else
        *is_evil = false;
    
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
    char* PORT_STRING = getenv("CHALPORT");
    if (PORT_STRING == NULL) {
        perror("No port defined.\n");
        return 1;
    }
    PORT = atoi(PORT_STRING);
    if (PORT <= 0) {
        perror("Port is not a number.\n");
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
    bool is_evil = false;

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
        memset(return_str, 0, sizeof(return_str)); // TODO is this line even necessary for any of them?
        ssize_t bytes_received = recvfrom(raw_sock, buffer, sizeof(buffer), 0,
                                          (struct sockaddr *)&src_addr, &addr_len);

        if (bytes_received <= 0)
            continue;

        uint8_t* sin_bytes = &(src_addr.sin_addr);
        printf("Recived packet from %d.%d.%d.%d\n", sin_bytes[0], sin_bytes[1], sin_bytes[2], sin_bytes[3]);
            
        int port = get_packet_data(buffer, &data_offset, &data_len, &is_evil, &connection_port);
        if (port != PORT)
            continue;
        // printf("Packet recieved on port %d with length %d!\n", port, data_len);

        src_addr.sin_port = connection_port;

        process_packet(buffer + data_offset, return_str, sizeof(return_str), is_evil);
        sendto(udp_sock, return_str, strlen(return_str), 0, (struct sockaddr *)&src_addr, addr_len);
        // printf("Sending string: %s\n", return_str);
    }

    close(raw_sock);
    close(udp_sock);
    return 0;
}
