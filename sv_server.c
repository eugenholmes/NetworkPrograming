#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

struct Student {
    char mssv[12];
    char hoTen[50];
    char ngaySinh[11];
    float diemTrungBinh;
};

int main(int argc, char *argv[]) {

    int port = atoi(argv[1]);

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

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

        struct Student sv;
        int len = recv(client, &sv, sizeof(sv), 0);
        
        if (len > 0) {
            char *client_ip = inet_ntoa(client_addr.sin_addr);

            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

            FILE *f_log = fopen("sv_log.txt", "a");
            if (f_log != NULL) {
                fprintf(f_log, "%s %s %s %s %s %.2f\n", client_ip, time_str, sv.mssv, sv.hoTen, sv.ngaySinh, sv.diemTrungBinh);
                printf("Received student information from %s in %s: MSSV=%s, Họ tên=%s, Ngày sinh=%s, Điểm TB=%.2f\n", client_ip, time_str, sv.mssv, sv.hoTen, sv.ngaySinh, sv.diemTrungBinh);
                fclose(f_log);
            } else {
                perror("Can not open log file");
            }
        }

        close(client);
    }

    close(listener);
    return 0;
}