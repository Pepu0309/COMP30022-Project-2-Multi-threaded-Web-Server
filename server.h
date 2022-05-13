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

#include <assert.h>

#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define REQUEST_MAX_BUFFER_SIZE 2000
#define INITIAL_STRING_SPACE 20

#define NULL_TERMINATOR_SPACE 1
#define BOOL_TRUE 1

#define FILE_EXTENSION_DELIMITER '.'
#define HTML_EXTENSION ".html"
#define JPEG_EXTENSION ".jpg"
#define CSS_EXTENSION ".css"
#define JAVA_SCRIPT_EXTENSION ".js"

#define SUCCESS "200 OK"
#define PAGE_NOT_FOUND "404 NOT FOUND"

void write_message(int socket, char *message);

void send_http_response(int socket, char *file_path);

char *parse_request_path(char *request_line);

void get_file_path(char **file_path, char *web_path_root, char *request_buffer);

void write_content_type(int sockfd_to_send, char *file_path);

#endif //COMP30023_2022_PROJECT_2_SERVER_H
