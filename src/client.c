#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAXLINE 1024

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server-ip> <port>\n", argv[0]);
        exit(1);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) die("socket");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0)
        die("inet_pton");

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        die("connect");

    printf("Connected to server. Type messages below.\n");

    char buf[MAXLINE];

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if (write(sockfd, buf, strlen(buf)) < 0)
            die("write");

        ssize_t n = read(sockfd, buf, sizeof(buf) - 1);
        if (n <= 0) {
            printf("Server closed connection.\n");
            break;
        }

        buf[n] = '\0';
        printf("%s", buf);
    }

    close(sockfd);
    return 0;
}
