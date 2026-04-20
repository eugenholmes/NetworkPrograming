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
#include <poll.h>

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
    
    struct pollfd fds[64];
    int nfds = 1;
    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char *client_ids[64];
    for (int i = 0; i < 64; i++) client_ids[i] = NULL;

    char buf[256];

    char *welcome = "Dang ky theo cu phap client_id: client_name\n";

    while (1) {
        int ret = poll(fds, nfds, -1);

        if (ret < 0) {
            perror("poll() failed");
            break;
        }

        if (fds[0].revents & POLLIN) {
            int client = accept(listener, NULL, NULL);

            if (nfds < 64) {
                printf("New client connected: %d\n", client);
                send(client, welcome, strlen(welcome), 0);
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;
            } else {
                close(client);
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0) {
                    printf("Client %s disconnected\n", client_ids[i]);
                    if (client_ids[i]) {
                        free(client_ids[i]);
                        client_ids[i] = NULL;
                    }
                    close(fds[i].fd);

                    for (int k = i; k < nfds - 1; k++) {
                        fds[k] = fds[k + 1];
                        client_ids[k] = client_ids[k + 1];
                    }

                    nfds--;
                    fds[nfds].fd = 0;
                    fds[nfds].events = 0;
                    client_ids[nfds] = NULL;

                    i--;
                    continue;
                } else {
                    buf[ret] = 0;
                    buf[strcspn(buf, "\r\n")] = 0;

                    if (client_ids[i] == NULL) {
                        char id[50], name[50];
                        if (sscanf(buf, "%[^:]: %s", id, name) == 2) {
                            client_ids[i] = strdup(id);
                            printf("Client %s registered ID %s \n", name, id);

                            char msg[256];
                            sprintf(msg, "Chao %s, ban da dang ky voi ID %s thanh cong!\n", name, id);
                            send(fds[i].fd, msg, strlen(msg), 0);
                        } else {
                            char *err = "SAI CU PHAP! Vui long nhap lai [id: name]\n";
                            send(fds[i].fd, err, strlen(err), 0);
                        }
                    } else {
                        time_t now = time(NULL);
                        struct tm *t = localtime(&now);
                        char time_buf[30];
                        strftime(time_buf, sizeof(time_buf), "%Y/%m/%d %I:%M:%S%p", t);

                        char send_buf[512];
                        sprintf(send_buf, "%s %s: %s\n", time_buf, client_ids[i], buf);

                        for (int j = 1; j < nfds; j++) {
                            if (fds[j].fd != fds[i].fd && client_ids[j] != NULL) {
                                send(fds[j].fd, send_buf, strlen(send_buf), 0);
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