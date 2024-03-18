#include <stdio.h>

#include <stdbool.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void printUsage(char *argv[]) {
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n  - create new database file\n");
    printf("\t -f  - (required) path to database file\n");
    return;
}

int main(int argc, char *argv[]) {
    int c, dbfd = -1;
    char* filepath = NULL;
    bool newfile = false;
    dbheader_t header;

    while ((c = getopt(argc, argv, "nf:")) != -1) {
        switch (c) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case '?':
                printf("Unknown option -%c\n", c);
                break;
            default:
                return -1;
        }
    }

    if (filepath == NULL) {
        printf("Filepath is a required argument\n");
        printUsage(argv);
        return 0;
    }

    if (newfile) {
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return -1;
        }

        if (create_db_header(&header) == STATUS_ERROR) {
            printf("Failed to create database header\n");
            return STATUS_ERROR;
        }

        if (output_file(dbfd, &header) != 0) {
            printf("Failed to write new database header\n");
            return STATUS_ERROR;
        }
    } else {
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return -1;
        }

        if (validate_db_header(dbfd, &header) == STATUS_ERROR) {
            printf("Failed to validate database\n");
            return STATUS_ERROR;
        }
    }

    return 0;
}