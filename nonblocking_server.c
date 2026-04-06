/*******************************************************************************
* @file    non_blocking_server.c
* @brief   Mô tả ngắn gọn về chức năng của file
* @date    2026-03-31 07:10
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ctype.h>

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    // Chuyen socket listener sang non-blocking
    unsigned long ul = 1;
    ioctl(listener, FIONBIO, &ul);

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

    // Server is now listening for incoming connections
    printf("Server is listening on port 8080...\n");

    int clients[64];
    int state[64];
    char email[64][256];
    int nclients = 0;

    char buf[256];
    int len;

    while (1) {
        // Chap nhan ket noi
        int client = accept(listener, NULL, NULL);
        if (client == -1) {
            if (errno == EWOULDBLOCK) {
                // Loi do dang cho ket noi
                // Bo qua
            } else {
                // Loi khac
            }
        } else {
            printf("New client accepted: %d\n", client);
            clients[nclients] = client;
            state[nclients] = 0; // Trang thai ban dau
            nclients++;
            ul = 1;
            ioctl(client, FIONBIO, &ul);
            send(client, "Ho ten:\n", 8, 0);
        }

        // Nhan du lieu tu cac client
        for (int i = 0; i < nclients; i++) {
            len = recv(clients[i], buf, sizeof(buf), 0);
            if (len == -1) {
                if (errno == EWOULDBLOCK) {
                    // Loi do cho du lieu
                    // Bo qua 
                } else {
                    continue;
                }
            } else {
                if (len == 0)
                    continue;
                buf[len] = 0;
                buf[strcspn(buf, "\r\n")] = 0; // Loai bo newline

                char copy[256];
                strncpy(copy, buf, sizeof(copy) - 1);
                int count = 0;
                if (state[i] == 0) {
                    state[i] = 1;
                    char* words[10];
                    char* token = strtok(copy, " ");
                    while (token != NULL && count < 10) {
                        words[count++] = token;
                        token = strtok(NULL, " ");
                    }
                    char* ten = words[count - 1];
                    strcpy(email[i], ten);
                    int email_len = strlen(email[i]);
                    for (int j = 0; j < count - 1; j++) {
                        email[i][email_len++] = words[j][0];
                    }
                    email[i][email_len] = '\0'; // Kết thúc chuỗi
                
                    send(clients[i], "MSSV:\n", 6, 0);
                } else if (state[i] == 1) {
                    state[i] = 0;
                    int mssv_len = strlen(buf);
                    const char* mssv_cuoi = (mssv_len > 6) ? (buf + mssv_len - 6) : buf;
                    strcat(email[i], mssv_cuoi);
                    strcat(email[i], "@sis.hust.edu.vn");
                    for (int j = 0; email[i][j]; j++) {
                        email[i][j] = tolower(email[i][j]);
                    }
                    send(clients[i], email[i], strlen(email[i]), 0);
                }
            }

        }
    }

    close(listener);
    return 0;
}