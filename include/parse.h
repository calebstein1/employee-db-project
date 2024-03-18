#ifndef EMP_DB_PROJ_PARSE_H
#define EMP_DB_PROJ_PARSE_H

#define HEADER_MAGIC 0x4c4c4144

typedef struct {
    unsigned int magic;
    unsigned short version;
    unsigned short count;
    unsigned int filesize;
} dbheader_t;

typedef struct {
    char name[256];
    char address[256];
    unsigned int hours;
} employee_t;

int create_db_header(int fd, dbheader_t *header);
int validate_db_header(int fd, dbheader_t *header);
int read_employees(int fd, dbheader_t *header, employee_t **employeesOut);
int output_file(int fd, dbheader_t *header);

#endif //EMP_DB_PROJ_PARSE_H
