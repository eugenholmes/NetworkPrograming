#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

int main (int argc, char *argv[]) {

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server_port);
    addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect() failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to %s:%d\n", server_ip, server_port);
    char recv_buf[1024];
    int bytes_received = recv(client, recv_buf, sizeof(recv_buf) - 1, 0);
    if (bytes_received > 0) {
        printf("Server: %s\n", recv_buf);
    } else {
        perror("recv() failed");
    }
    printf("Enter content (type 'exit' to quit):\n");

    char buffer[1024];
    while (1) {
        printf("> ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;

        if (strcmp(buffer, "exit\n") == 0) {
            break;
        }

        int bytes_sent = send(client, buffer, strlen(buffer), 0);
        if (bytes_sent < 0) {
            perror("send() failed");
            break;
        }
    }

    close(client);

    return 0;
}