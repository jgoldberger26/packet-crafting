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
#define ACTION_SET_PWD 2

struct app_data
{
    uint8_t action;
    char sdata[255];
};

char passwords[255][255];
uint8_t password_idx = 0;

void addAdminPwd(char *sdata, uint8_t slen)
{
    memset(passwords[password_idx], 0, 255);
    memmove(passwords[password_idx], sdata, slen);
    password_idx += 1; // Will overflow and reset on its own
}

bool checkPwd(char *sdata, uint8_t slen)
{
    for (int i = 0; i < 256; i++)
    {
        if (strncmp(sdata, passwords[i], slen) == 0)
        {
            return true;
        }
    }
    return false;
}

void process_packet(uint8_t *buffer, ssize_t len, struct sockaddr_in *src_addr, char *str_buf, size_t str_len)
{
    struct app_data *dat = (struct app_data *)buffer;

    uint8_t action = dat->action;
    char *sdata = dat->sdata;
    size_t slen = strlen(sdata);

    if (slen >= 255)
    {
        slen = 254; // No overreads!
    }

    switch (action)
    {
    case ACTION_PING:
        strncpy(str_buf, sdata, str_len);
        str_buf[slen] = '\0';
        // printf("Pong!");
        return;
    case ACTION_GET_FLAG:
        if (checkPwd(sdata, slen))
        {
            strncpy(str_buf, "Welcome, admin! Flag: ", str_len);
            strcat(str_buf, FLAG);
            strcat(str_buf, "\n");
            return;
        }
        strncpy(str_buf, "Incorrect password!\n\0", str_len);
        return;
    case ACTION_SET_PWD:
        if (src_addr->sin_addr.s_addr != htonl(ADMIN_IP))
        {
            strncpy(str_buf, "I don't recognize your ip...\n\0", str_len);
            return;
        }
        addAdminPwd(sdata, slen);
        strncpy(str_buf, "Welcome admin! Password set!\n\0", str_len);
        return;
    default:
        strncpy(str_buf, "Invalid action!\n\0", str_len);
        return;
    }
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
