//
// Created by User on 14/5/2022.
//

#ifndef COMP30023_2022_PROJECT_2_PARSE_H
#define COMP30023_2022_PROJECT_2_PARSE_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NULL_TERMINATOR_SPACE 1
#define TWO_SPACES 2

#define SAME_STRING 0

#define GET_REQUEST "GET"
#define PROTOCOL_VER "HTTP/1.0"

bool parse_request_path(char *request_buffer, char **request_path);

bool get_file_path(char **file_path, char *web_path_root, char *request_buffer);

bool check_escape_request_path(char *request_path);

#endif //COMP30023_2022_PROJECT_2_PARSE_H
