#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "%s  by darkQ\n", argv[0]);
        fprintf(stderr, "Usage: %s <file> <port>\n", argv[0]);
        return 1;
    }

    int filefd = open(argv[1], O_RDONLY);
    if (filefd < 0) { perror("open"); return 1; }

    struct stat st;
    fstat(filefd, &st);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(argv[2]));
    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(sockfd, 1);

    while (1) {
        int client = accept(sockfd, NULL, NULL);
        char buf[BUF_SIZE];
        read(client, buf, BUF_SIZE); // Read request (not parsed)

        dprintf(client,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/octet-stream\r\n"
            "Content-Length: %ld\r\n"
            "Content-Disposition: attachment; filename=\"%s\"\r\n"
            "\r\n",
            st.st_size, strrchr(argv[1], '/') ? strrchr(argv[1], '/')+1 : argv[1]);

        lseek(filefd, 0, SEEK_SET);
        ssize_t n;
        while ((n = read(filefd, buf, BUF_SIZE)) > 0)
            write(client, buf, n);

        close(client);
    }
    close(filefd);
    close(sockfd);
    return 0;
}
