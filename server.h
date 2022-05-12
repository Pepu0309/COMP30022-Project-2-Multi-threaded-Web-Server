//
// Created by User on 12/5/2022.
//

#ifndef COMP30023_2022_PROJECT_2_SERVER_H
#define COMP30023_2022_PROJECT_2_SERVER_H

#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

#define REQUEST_MAX_BUFFER_SIZE 2000
#define INITIAL_STRING_SPACE 20
#define NULL_TERMINATOR_SPACE 1

char *parse_request_path(char *request_line);

void get_file_path(char **file_path, char *web_path_root, char *request_buffer);

#endif //COMP30023_2022_PROJECT_2_SERVER_H
