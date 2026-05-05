#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAXLINE 1024

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);

    int port = atoi(argv[1]);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) die("socket");

    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        die("setsockopt");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        die("bind");

    if (listen(listenfd, 10) < 0)
        die("listen");

    printf("Server listening on port %d\n", port);

    fd_set master_set, read_set;
    FD_ZERO(&master_set);
    FD_SET(listenfd, &master_set);

    int maxfd = listenfd;

    while (1) {
        read_set = master_set;

        if (select(maxfd + 1, &read_set, NULL, NULL, NULL) < 0)
            die("select");

        for (int fd = 0; fd <= maxfd; fd++) {
            if (!FD_ISSET(fd, &read_set)) continue;

            if (fd == listenfd) {
                struct sockaddr_in clientaddr;
                socklen_t clientlen = sizeof(clientaddr);

                int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
                if (clientfd < 0) {
                    perror("accept");
                    continue;
                }

                FD_SET(clientfd, &master_set);
                if (clientfd > maxfd) maxfd = clientfd;

                printf("New connection from %s:%d\n",
                       inet_ntoa(clientaddr.sin_addr),
                       ntohs(clientaddr.sin_port));
            } else {
                char buf[MAXLINE];
                ssize_t n = read(fd, buf, sizeof(buf) - 1);

                if (n <= 0) {
                    printf("Client disconnected: fd %d\n", fd);
                    close(fd);
                    FD_CLR(fd, &master_set);
                } else {
                    buf[n] = '\0';

                    printf("Received from fd %d: %s", fd, buf);

                    char response[MAXLINE + 64];
                    snprintf(response, sizeof(response), "Echo from server: %s", buf);

                    if (write(fd, response, strlen(response)) < 0) {
                        perror("write");
                        close(fd);
                        FD_CLR(fd, &master_set);
                    }
                }
            }
        }
    }

    close(listenfd);
    return 0;
}
