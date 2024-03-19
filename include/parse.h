#ifndef EMP_DB_PROJ_PARSE_H
#define EMP_DB_PROJ_PARSE_H

#define HEADER_MAGIC 0x4c4c4144

struct dbheader_t {
    unsigned int magic;
    unsigned short version;
    unsigned short count;
    unsigned int filesize;
};

struct employee_t {
    char name[256];
    char address[256];
    unsigned int hours;
    int id;
};

int create_db_header(struct dbheader_t *header);
int validate_db_header(int fd, struct dbheader_t *header);
int read_employees(int fd, struct dbheader_t *header, struct employee_t **employees_out);
int add_employee(struct dbheader_t *header, struct employee_t *employees, char *add_string);
void print_employees(struct dbheader_t *header, struct employee_t *employees);
void query_employees(struct dbheader_t *header, struct employee_t *employees, char *query_string);
void delete_employee(struct employee_t *employees, int delete_id);
int output_file(int fd, struct dbheader_t *header, struct employee_t *employees);
int check_input(char *str);

#endif //EMP_DB_PROJ_PARSE_H
