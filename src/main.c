#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n  - create new database file\n");
    printf("\t -f  - (required) path to database file\n");
    return;
}

int main(int argc, char *argv[]) {
    int c, dbfd = -1;
    char *file_path, *add_string, *query_string, *delete_id;
    bool new_file, list_employees;
    struct dbheader_t header;
    struct employee_t *employees;
    file_path = add_string = query_string = delete_id = NULL;
    new_file = list_employees = false;

    while ((c = getopt(argc, argv, "nf:a:lq:r:")) != -1) {
        switch (c) {
            case 'n':
                new_file = true;
                break;
            case 'f':
                file_path = optarg;
                break;
            case 'a':
                add_string = optarg;
                break;
            case 'l':
                list_employees = true;
                break;
            case 'q':
                query_string = optarg;
                break;
            case 'r':
                delete_id = optarg;
                break;
            case '?':
                printf("Unknown option -%c\n", c);
                break;
            default:
                return -1;
        }
    }

    if (file_path == NULL) {
        printf("Filepath is a required argument\n");
        print_usage(argv);
        return 0;
    }

    if (new_file) {
        dbfd = create_db_file(file_path);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return -1;
        }

        if (create_db_header(&header) == STATUS_ERROR) {
            printf("Failed to create database header\n");
            return STATUS_ERROR;
        }
    } else {
        dbfd = open_db_file(file_path);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return -1;
        }

        if (validate_db_header(dbfd, &header) == STATUS_ERROR) {
            printf("Failed to validate database\n");
            return STATUS_ERROR;
        }
    }

    if (read_employees(dbfd, &header, &employees) != STATUS_SUCCESS) {
        printf("Failed to read employees\n");
        return STATUS_ERROR;
    }

    if (add_string) {
        if (check_input(add_string) != 0) {
            printf("Name or address exceeded allowed length\n");
            return STATUS_ERROR;
        }
        employees = realloc(employees, ++header.count * sizeof(struct employee_t));
        add_employee(&header, employees, add_string);
    }

    if (list_employees) {
        print_employees(&header, employees);
    }

    if (query_string) {
        query_employees(&header, employees, query_string);
    }

    if (delete_id) {
        int delete_id_int;
        if ((delete_id_int = atoi(delete_id)) == 0 || delete_id_int > header.count) {
            printf("Must supply a single valid id to delete\n");
            return STATUS_ERROR;
        }
        delete_employee(employees, delete_id_int);
    }

    if (output_file(dbfd, &header, employees) != 0) {
        printf("Failed to write new database header\n");
        return STATUS_ERROR;
    }

    free(employees);
    return 0;
}