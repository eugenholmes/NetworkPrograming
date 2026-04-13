#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

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

    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(listener, &fdread);

    ClientState clients[1024];
    for (int i = 0; i < 1024; i++) {
        clients[i].fd = -1;
        clients[i].is_authenticated = 0;
    }

    int max_fd = listener;    
    char buf[1024];

    char *login = "Nhap username va password: ";

    while (1) {
        fdtest = fdread;
        int ret = select(max_fd + 1, &fdtest, NULL, NULL, NULL);

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &fdtest)) {
                if (i == listener) {
                    int client = accept(listener, NULL, NULL);
                    if (client != -1 && client < FD_SETSIZE) {
                        printf("New client connected %d\n", client);
                        FD_SET(client, &fdread);
                        clients[client].fd = client;
                        clients[client].is_authenticated = 0;
                        send(client, login, strlen(login), 0);
                        if (client > max_fd) max_fd = client;
                    } else {
                        close(client);
                    }
                } else {
                    ret = recv(i, buf, sizeof(buf) - 1, 0);
                    if (ret <= 0) {
                        printf("Client %d disconnected\n", i);
                        FD_CLR(i, &fdread);
                        close(i);
                        clients[i].fd = -1;
                        continue;
                    }
                    buf[ret] = 0;

                    if (buf[ret-1] == '\n') buf[ret-1] = 0;
                    if (buf[ret-2] == '\r') buf[ret-2] = 0;

                    if (clients[i].is_authenticated == 0) {
                        char user[50], pass[50];
                        if (sscanf(buf, "%s %s", user, pass) == 2) {
                            if (check_login(user, pass)) {
                                clients[i].is_authenticated = 1;
                                send(i, "Dang nhap thanh cong! Nhap lenh: \n", 34, 0);
                            } else {
                                send(i, "Sai user hoac pass. Nhap lai: ", 30, 0);
                            }
                        } else {
                            send(i, "Sai cu phap. Nhap [user pass]: ", 31, 0);
                        }
                    } else {
                        char cmd[1024 + 20];
                        sprintf(cmd, "%s > out%d.txt", buf, i);
                        system(cmd);

                        FILE *f = fopen(cmd + strlen(buf) + 3, "r");
                        if (f) {
                            while (fgets(buf, sizeof(buf), f)) {
                                send(i, buf, strlen(buf), 0);
                            }
                            fclose(f);
                        }
                        send(i, "\nDone.\n", 7, 0);
                    }
                }
            }
        }
    }
    return 0;
}