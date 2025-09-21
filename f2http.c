/* f2http.c
   File to HTTP download server
   by darkQ
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

#define BUF_SIZE 262144

void send_response(int client, int fd, off_t filesize, const char *filename, off_t start, off_t end) {
    char header[1024];
    int header_len;
    off_t to_send = end - start + 1;

    header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 206 Partial Content\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Disposition: attachment; filename=\"%s\"\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Range: bytes %ld-%ld/%ld\r\n"
        "Content-Length: %ld\r\n"
        "\r\n",
        filename, start, end, filesize, to_send);

    write(client, header, header_len);
    lseek(fd, start, SEEK_SET);
    off_t sent = 0;
    while (sent < to_send) {
        ssize_t n = sendfile(client, fd, NULL, to_send - sent);
        if (n <= 0) break;
        sent += n;
    }
}

void send_full_response(int client, int fd, off_t filesize, const char *filename) {
    char header[1024];
    int header_len;

    header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Disposition: attachment; filename=\"%s\"\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: %ld\r\n"
        "\r\n",
        filename, filesize);

    write(client, header, header_len);
    off_t sent = 0;
    while (sent < filesize) {
        ssize_t n = sendfile(client, fd, NULL, filesize - sent);
        if (n <= 0) break;
        sent += n;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "%s\n by darkQ\n\n Syntax: %s <file> <port>\n", argv[0],argv[0]);
        return 1;
    }

    const char *filepath = argv[1];
    int port = atoi(argv[2]);
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        return 1;
    }
    off_t filesize = st.st_size;

    const char *filename = strrchr(filepath, '/');
    filename = filename ? filename + 1 : filepath;

    int server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server, 5);

    while (1) {
        int client = accept(server, NULL, NULL);
        if (client < 0) continue;

        char buf[BUF_SIZE] = {0};
        read(client, buf, sizeof(buf) - 1);

        if (strncmp(buf, "GET ", 4) == 0) {
            char *range = strstr(buf, "Range: bytes=");
            if (range) {
                off_t start = 0, end = filesize - 1;
                sscanf(range, "Range: bytes=%ld-%ld", &start, &end);
                if (end >= filesize) end = filesize - 1;
                if (start > end) start = end;
                send_response(client, fd, filesize, filename, start, end);
            } else {
                send_full_response(client, fd, filesize, filename);
            }
        }
        close(client);
    }
    close(fd);
    return 0;
}
