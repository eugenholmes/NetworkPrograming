#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    int port = atoi(argv[1]);
    char *hello = argv[2];
    char *data = argv[3];

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    if (listen(listener, 5)) {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
        
        if (client < 0) {
            perror("accept() failed");
            continue;
        }
        printf("Client %s\n", inet_ntoa(client_addr.sin_addr));

        FILE *f_hello = fopen(hello, "r");
        if (f_hello != NULL) {
            char hello_buffer[2048];
            while (fgets(hello_buffer, sizeof(hello_buffer), f_hello) != NULL) {
                send(client, hello_buffer, strlen(hello_buffer), 0);
            }
            fclose(f_hello);
        } else {
            char *msg = "Hello Client!\n";
            send(client, msg, strlen(msg), 0);
        }

        FILE *f_out = fopen(data, "wb");
        if (f_out == NULL) {
            perror("file open failed");
            close(client);
            continue;
        }

        char buf[2048];
        int len;
        while ((len = recv(client, buf, sizeof(buf), 0)) > 0) {
            fwrite(buf, 1, len, f_out);
        }

        printf("Received data and saved to %s\n", data);
        
        fclose(f_out);
        close(client);
    }

    close(listener);
    return 0;
}