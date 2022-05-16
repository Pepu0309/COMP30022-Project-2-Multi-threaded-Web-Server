//
// Created by User on 14/5/2022.
//

#ifndef COMP30023_2022_PROJECT_2_RESPOND_H
#define COMP30023_2022_PROJECT_2_RESPOND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>

#define FILE_EXTENSION_DELIMITER '.'
#define HTML_EXTENSION ".html"
#define JPEG_EXTENSION ".jpg"
#define CSS_EXTENSION ".css"
#define JAVA_SCRIPT_EXTENSION ".js"

#define ZERO_OFFSET 1
#define NULL_TERMINATOR_SPACE 1

#define SAME_STRING 0

void write_message(int socket, char *message);

void send_http_response(int socket, char *file_path);

void write_content_type(int sockfd_to_send, char *file_path);

bool check_escape_file_path(char *file_path);

#endif //COMP30023_2022_PROJECT_2_RESPOND_H
