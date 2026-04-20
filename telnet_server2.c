#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <poll.h>

typedef struct {
    int fd;
    int is_authenticated; // 0: Chưa đăng nhập, 1: Đã đăng nhập
} ClientState;

int check_login(char *user, char *pass) {
    FILE *f = fopen("databases.txt", "r");
    if (f == NULL) return 0;

    char f_user[50], f_pass[50];
    while (fscanf(f, "%s %s", f_user, f_pass) != EOF) {
        if (strcmp(user, f_user) == 0 && strcmp(pass, f_pass) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

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

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    printf("Telnet Server dang chay tren port 8080...\n");

    struct pollfd fds[1024];
    int nfds = 1;
    fds[0].fd = listener;
    fds[0].events = POLLIN;

    ClientState clients[1024];
    for (int i = 0; i < 1024; i++) {
        clients[i].fd = -1;
        clients[i].is_authenticated = 0;
    }

    char buf[1024];

    char *login = "Nhap username va password: ";

    while (1) {
        int ret = poll(fds, nfds, -1);

        if (fds[0].revents & POLLIN) {
            int client = accept(listener, NULL, NULL);
            if (nfds < 1024) {
                printf("New client connected %d\n", client);
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;
                clients[nfds].fd = client;
                clients[nfds].is_authenticated = 0;
                send(client, login, strlen(login), 0);
            } else {
                close(client);
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0) {
                    printf("Client %d disconnected\n", fds[i].fd);
                    
                    close(fds[i].fd);

                    for (int k = i; k < nfds - 1; k++) {
                        fds[k] = fds[k + 1];
                        clients[k] = clients[k + 1];
                    }

                    nfds--;
                    fds[nfds].fd = 0;
                    fds[nfds].events = 0;
                    clients[nfds].fd = -1;
                    clients[nfds].is_authenticated = 0;

                    i--;
                    continue;
                } else {
                    buf[ret] = 0;

                    if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = 0;
                    if (buf[strlen(buf) - 1] == '\r') buf[strlen(buf) - 1] = 0;

                    if (clients[i].is_authenticated == 0) {
                        char user[50], pass[50];
                        if (sscanf(buf, "%s %s", user, pass) == 2) {
                            if (check_login(user, pass)) {
                                clients[i].is_authenticated = 1;
                                send(fds[i].fd, "Dang nhap thanh cong! Nhap lenh: \n", 34, 0);
                            } else {
                                send(fds[i].fd, "Sai user hoac pass. Nhap lai: ", 30, 0);
                            }
                        } else {
                            send(fds[i].fd, "Sai cu phap. Nhap [user pass]: ", 31, 0);
                        }
                    } else {
                        char cmd[2048];
                        sprintf(cmd, "%s > out%d.txt", buf, i);
                        system(cmd);

                        FILE *f = fopen(cmd + strlen(buf) + 3, "r");
                        if (f) {
                            while (fgets(buf, sizeof(buf), f)) {
                                send(fds[i].fd, buf, strlen(buf), 0);
                            }
                            fclose(f);
                        }
                        send(fds[i].fd, "\nDone.\n", 7, 0);
                    }
                }
            }
        }
    }
    return 0;
}