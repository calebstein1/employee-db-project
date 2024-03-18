#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "parse.h"

int create_db_header(struct dbheader_t *header) {
    header->magic = HEADER_MAGIC;
    header->version = 0x1;
    header->count = 0;
    header->filesize = sizeof(*header);

    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *header) {
    if (fd < 0) {
        printf("Got a bad file from user\n");
        return STATUS_ERROR;
    }

    header->magic = htonl(header->magic);
    header->version = htons(header->version);
    header->count = htons(header->count);
    header->filesize = htonl(header->filesize);

    lseek(fd, 0, SEEK_SET);
    write(fd, header, sizeof(*header));

    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t *header) {
    struct stat dbstat = {0};

    if (fd < 0) {
        printf("Got a bad file from user\n");
        goto ret_err;
    }
    if (fstat(fd, &dbstat) != 0) {
        perror("fstat");
        goto close_fd;
    }
    if (read(fd, header, sizeof(*header)) != sizeof(*header)) {
        perror("read");
        goto close_fd;
    }

    header->magic = ntohl(header->magic);
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->filesize = ntohl(header->filesize);

    if (header->magic != HEADER_MAGIC) {
        printf("Corrupted or not a db file\n");
        goto close_fd;
    }
    if (header->version != 1) {
        printf("Wrong db file version\n");
        goto close_fd;
    }
    if (header->filesize != dbstat.st_size) {
        printf("Corrupted db file\n");
        goto close_fd;
    }

    return STATUS_SUCCESS;

    close_fd:
        close(fd);
    ret_err:
        return STATUS_ERROR;
}
