#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <time.h>

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }
    
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) {
        perror("setsockopt() failed");
        close(listener);
        return 1;
    }
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);
    
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        close(listener);
        return 1;
    }
    
    if (listen(listener, 5)) {
        perror("listen() failed");
        close(listener);
        return 1;
    }
    
    printf("Server is listening on port 8080...\n");
    
    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(listener, &fdread);

    char *client_ids[FD_SETSIZE];
    for (int i = 0; i < FD_SETSIZE; i++) client_ids[i] = NULL;

    int max_fd = listener;
    char buf[256];

    char *welcome = "Dang ky theo cu phap client_id: client_name\n";

    while (1) {
        fdtest = fdread;
        int ret = select(max_fd + 1, &fdtest, NULL, NULL, NULL);

        if (ret < 0) {
            perror("select() failed");
            break;
        }

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &fdtest)) {
                if (i == listener) {
                    int client = accept(listener, NULL, NULL);
                    if (client != -1 && client < FD_SETSIZE) {
                        printf("New client connected %d\n", client);
                        FD_SET(client, &fdread);
                        send(client, welcome, strlen(welcome), 0);
                        if (client > max_fd) max_fd = client;
                    } else {
                        close(client);
                    }
                } else {
                    ret = recv(i, buf, sizeof(buf), 0);
                    if (ret <= 0) {
                        printf("Client %d disconnected\n", i);
                        if (client_ids[i]) {
                            free(client_ids[i]);
                            client_ids[i] = NULL;
                        }
                        FD_CLR(i, &fdread);
                        close(i);
                    } else {
                        buf[ret] = 0;
                        if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = 0;
                        if (buf[strlen(buf) - 1] == '\r') buf[strlen(buf) - 1] = 0;

                        if (client_ids[i] == NULL) {
                            char id[50], name[50];
                            if (sscanf(buf, "%[^:]: %s", id, name) == 2) {
                                client_ids[i] = strdup(id);
                                printf("Client %d registered as %s (ID: %s)\n", i, name, id);

                                char msg[256];
                                sprintf(msg, "Chao %s, ban da dang ky voi ID %s thanh cong!\n", name, id);
                                send(i, msg, strlen(msg), 0);
                            } else {
                                char *err = "SAI CU PHAP! Vui long nhap lai [id: name]\n";
                                send(i, err, strlen(err), 0);
                            }
                        } else {
                            time_t now = time(NULL);
                            struct tm *t = localtime(&now);
                            char time_buf[30];
                            strftime(time_buf, sizeof(time_buf), "%Y/%m/%d %I:%M:%S%p", t);

                            char send_buf[512];
                            sprintf(send_buf, "%s %s: %s\n", time_buf, client_ids[i], buf);

                            for (int j = 0; j < FD_SETSIZE; j++) {
                                if (FD_ISSET(j, &fdread) && j != listener && j != i && client_ids[j] != NULL) {
                                    send(j, send_buf, strlen(send_buf), 0);
                                }
                            }
                        }
                    }
                }            
            }
        }
    }

    close(listener);
    return 0;
}