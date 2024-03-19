#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
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

int read_employees(int fd, struct dbheader_t *header, struct employee_t **employees_out) {
    if (fd < 0) {
        printf("Got a bad file from user\n");
        return STATUS_ERROR;
    }

    int i;
    struct employee_t *employees;
    if ((employees = calloc(header->count, sizeof(struct employee_t))) == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    read(fd, employees, header->count * sizeof(struct employee_t));

    for (i = 0; i < header->count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
        employees[i].id = ntohl(employees[i].id);
    }

    *employees_out = employees;
    return STATUS_SUCCESS;
}

int add_employee(struct dbheader_t *header, struct employee_t *employees, char *add_string) {
    char *name = strtok(add_string, ",");
    char *addr = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");

    strcpy(employees[header->count - 1].name, name);
    strcpy(employees[header->count - 1].address, addr);
    employees[header->count - 1].hours = atoi(hours);
    employees[header->count - 1].id = header->count;

    return STATUS_SUCCESS;
}

void print_employees(struct dbheader_t *header, struct employee_t *employees) {
    int i;
    for (i = 0; i < header->count; i++) {
        printf("Employee %d:\n\tName: %s\n\tAddress: %s\n\tHours worked: %d\n\n",
               employees[i].id, employees[i].name, employees[i].address, employees[i].hours);
    }
}

void query_employees(struct dbheader_t *header, struct employee_t *employees, char *query_string) {
    int i;
    for (i = 0; i < header->count; i++) {
        if (memcmp(employees[i].name, query_string, strlen(employees[i].name)) == 0) {
            printf("Employee %d:\n\tName: %s\n\tAddress: %s\n\tHours worked: %d\n\n",
                   employees[i].id, employees[i].name, employees[i].address, employees[i].hours);
        }
    }
}

void delete_employee(struct employee_t *employees, int delete_id) {
    employees[delete_id - 1].id = -1;
}

int output_file(int fd, struct dbheader_t *header, struct employee_t *employees) {
    int i, real_count = header->count, next_id = 0;

    if (fd < 0) {
        printf("Got a bad file from user\n");
        return STATUS_ERROR;
    }

    header->magic = htonl(header->magic);
    header->version = htons(header->version);
    header->filesize = sizeof(struct employee_t) * real_count;

    lseek(fd, sizeof(*header), SEEK_SET);

    for (i = 0; i < real_count; i++) {
        if (employees[i].id == -1) {
            header->count--;
            header->filesize -= sizeof(struct employee_t);
            continue;
        }
        employees[i].hours = htonl(employees[i].hours);
        employees[i].id = htonl(++next_id);
        write(fd, &employees[i], sizeof(struct employee_t));
    }

    header->count = htons(header->count);
    header->filesize = htonl(header->filesize + sizeof(*header));
    lseek(fd, 0, SEEK_SET);
    write(fd, header, sizeof(*header));
    ftruncate(fd, ntohl(header->filesize));

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

int check_input(char *str) {
    int i, j = 0;
    for (i = 0; str[i] != ','; i++) {
        if (i > 255) {
            return STATUS_ERROR;
        }
    }
    for (i++; str[i] != ','; i++) {
        if (j > 255) {
            return STATUS_ERROR;
        }
        j++;
    }

    return STATUS_SUCCESS;
}
