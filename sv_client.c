#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

struct Student {
    char mssv[12];
    char hoTen[50];
    char ngaySinh[11];
    float diemTrungBinh;
};

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

    struct Student sv;
    memset(&sv, 0, sizeof(sv));

    printf("Nhập thông tin sinh viên\n");
    
    printf("MSSV: ");
    scanf("%s", sv.mssv);
    getchar();

    printf("Họ tên: ");
    fgets(sv.hoTen, sizeof(sv.hoTen), stdin);
    sv.hoTen[strcspn(sv.hoTen, "\n")] = 0;

    printf("Ngày sinh: ");
    scanf("%s", sv.ngaySinh);

    printf("Điểm trung bình: ");
    scanf("%f", &sv.diemTrungBinh);
    
    int bytes_sent = send(client, &sv, sizeof(sv), 0);
    if (bytes_sent > 0) {
        printf("Student information sent to server successfully.\n");
    } else {
        perror("send() failed");
        exit(EXIT_FAILURE);
    }

    close(client);

    return 0;
}