#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[]) {  
    int port_s = atoi(argv[1]);
    char *ip_d = argv[2];
    int port_d = atoi(argv[3]);

    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(port_s);

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));

    unsigned long ul = 1;
    ioctl(sockfd, FIONBIO, &ul);

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(ip_d);
    dest_addr.sin_port = htons(port_d);

    char buf[1024];

    while (1) {
        int bytes_waiting;
        ioctl(0, FIONREAD, &bytes_waiting);

        if (bytes_waiting > 0) {
            if (fgets(buf, sizeof(buf), stdin) != NULL) {
                sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            }
        }

        struct sockaddr_in sender_addr;
        socklen_t sender_addr_len = sizeof(sender_addr);
        int ret = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
        if (ret > 0) {
            buf[ret] = '\0';
            if (buf[ret - 1] == '\n') {
                buf[ret - 1] = '\0';
            }
            printf("\r%s:%d said: %s\n", inet_ntoa(sender_addr.sin_addr), ntohs(sender_addr.sin_port), buf);
            fflush(stdout);
        }

    }

    close(sockfd);
    return 0;
}