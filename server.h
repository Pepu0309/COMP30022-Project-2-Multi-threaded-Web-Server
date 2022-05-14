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

#include <stdbool.h>
#include <assert.h>
#include <pthread.h>

#include "parse.h"
#include "respond.h"

#define IMPLEMENTS_IPV6
#define MULTITHREADED

#define REQUEST_MAX_BUFFER_SIZE 2000
#define IPV4_ARG "4"
#define IPV6_ARG "6"

#define NULL_TERMINATOR_SPACE 1
#define ZERO_OFFSET 1

typedef struct serve_connection_args serve_connection_args_t;
struct serve_connection_args {
    int newsockfd;
    char *web_root_path;
};

void *serve_connection(void *serve_connection_args);

#endif //COMP30023_2022_PROJECT_2_SERVER_H
