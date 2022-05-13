// A simple server in the internet domain using TCP
// The port number is passed as an argument
// To compile: gcc server.c -o server
// Reference: Beej's networking guide, man pages
#include "server.h"

int main(int argc, char** argv) {
	int sockfd, newsockfd, n, re, s;
	char buffer[REQUEST_MAX_BUFFER_SIZE];
	struct addrinfo hints, *res;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;

	if (argc < 4) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(EXIT_FAILURE);
	}

    char *web_path_root = argv[3];

	// Create address we're going to listen on (with given port number)
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;       // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept
	// node (NULL means any interface), service (port), hints, res. Note: argv[2] is the port_number
	s = getaddrinfo(NULL, argv[2], &hints, &res);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	// Create socket
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Reuse port if possible
	re = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    // Bind address to the socket
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(res);

    while(true) {
        // Listen on socket - means we're ready to accept connections,
        // incoming connection requests will be queued, man 3 listen
        if (listen(sockfd, 5) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        // Accept a connection - blocks until a connection is ready to be accepted
        // Get back a new file descriptor to communicate on
        client_addr_size = sizeof client_addr;
        newsockfd =
            accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_size);
        if (newsockfd < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }


        // Read characters from the connection, then process
        n = read(newsockfd, buffer, REQUEST_MAX_BUFFER_SIZE); // n is number of characters read
        if (n < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        // Null-terminate string
        buffer[n] = '\0';

        char *file_path;
        get_file_path(&file_path, web_path_root, buffer);
        printf("%s", file_path);

        send_http_response(newsockfd, file_path);

        free(file_path);
        close(newsockfd);
    }
    close(sockfd);
	return 0;
}

void write_message(int sockfd_to_send, char *message) {
    // Write message back
    // printf("Here is the message: %s\n", buffer);
    printf("Writing message");
    int n = write(sockfd_to_send, message, strlen(message));
    if (n < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
}

void send_http_response(int sockfd_to_send, char *file_path) {
    int file_path_fd;

    // stat struct from standard library which will allow access to the file size
    struct stat file_stat;

    // If the file we're trying to read from does not exist, open will return -1 as per the linux manual located at
    // https://man7.org/linux/man-pages/man2/open.2.html. Hence, if we cannot open what is located at the file path
    // or we find a "../" defined as the constant ESCAPE_PATH in the file_path, then we return a 404.
    if((file_path_fd = open(file_path, O_RDONLY)) < 0 || strstr(file_path, "/../") != NULL) {
        write_message(sockfd_to_send, "HTTP/1.0 404 Not Found\r\n\r\n");
    // Otherwise, the file exists, and we can use fstat to get the statistics of it.
    } else {
        /* Call fstat on file_path to get the statistics of the file located at file_path and then store it in the
           stat struct file_stat declared earlier. */
        fstat(file_path_fd, &file_stat);

        // Test that the file_path leads to a regular file and not something else like a directory.
        if(S_ISREG(file_stat.st_mode)) {
            write_message(sockfd_to_send, "HTTP/1.0 200 OK\r\n");
            write_message(sockfd_to_send, "Content-Type: ");
            write_content_type(sockfd_to_send, file_path);
            write_message(sockfd_to_send, "\r\n\r\n");

            off_t file_to_send_size = file_stat.st_size;
            off_t total_num_bytes_sent = 0;
            off_t bytes_successfully_sent = 0;

            // Track the bytes sent by sendfile() and make sure that all bytes are sent.
            while(total_num_bytes_sent < file_to_send_size) {
                // sendfile returns -1 in the case of an error or the number of bytes successfully sent as per the
                // linux manual located at https://man7.org/linux/man-pages/man2/sendfile.2.html.
                bytes_successfully_sent = sendfile(sockfd_to_send, file_path_fd,
                                                   &total_num_bytes_sent, file_to_send_size);
                // If there was no error, then we increment the total number of bytes sent.
                if(bytes_successfully_sent >= 0) {
                    total_num_bytes_sent += bytes_successfully_sent;
                }
            }
        // Otherwise, we send back a 404 not found response as well.
        } else {
            write_message(sockfd_to_send, "HTTP/1.0 404 Not Found\r\n\r\n");
        }

    }

}

char *parse_request_path(char *request_buffer) {
    // Get the request line from the buffer.
    char *request_line = strtok(request_buffer, "\r\n");

    // Consume the GET which is the first token of strtok
    strtok(request_line, " ");

    // Call strtok again using NULL as first argument to get the second token which is the file from the request and
    // return it.
    return strtok(NULL, " ");
}

void get_file_path(char **file_path, char *web_path_root, char *request_buffer) {
    char *request_path = parse_request_path(request_buffer);
    size_t file_path_length = strlen(web_path_root) + strlen(request_path) + NULL_TERMINATOR_SPACE;

    *file_path = (char *) malloc (file_path_length * sizeof(char));

    /* Concatenate both the web_path_root and request_path into a new string variable. I was looking for a way
       concatenate them into a new string variable with enough space instead of just concatenating request_path to
       web_path_root and came across this code from a stackoverflow post.
       https://stackoverflow.com/questions/8465006/how-do-i-concatenate-two-strings-in-c
       and used it as reference. While the rest of the code is fairly similar, I wrote it before coming across the
       post. */

    /* Copy web_path_root to file_path first since it has enough space to store both strings. */
    strcpy(*file_path, web_path_root);
    /* Then concatenate the request_path to file_path which should already contain web_path_root. */
    strcat(*file_path, request_path);
}

void write_content_type(int sockfd_to_send, char *file_path) {
    char *extension;

    // Use strrchr to get the last occurrence of the FILE_EXTENSION_DELIMITER which is the '.' character. This deals
    // with "false" extensions in the file_path. Handling '.' characters that are not associated with a file path when
    // there is no extension is handled below.
    extension = strrchr(file_path, FILE_EXTENSION_DELIMITER);

    // If there is a '.' character found in the file_path
    if(extension != NULL) {
        // Among the four MIME content type the server identifies, if any of them are found, then return the MIME
        // content type as specified by https://mimetype.io/all-types/.
        if(strcmp(extension, HTML_EXTENSION) == 0) {
            write_message(sockfd_to_send, "text/html");
        } else if (strcmp(extension, JPEG_EXTENSION) == 0) {
            write_message(sockfd_to_send, "image/jpeg");
        } else if (strcmp(extension, JAVA_SCRIPT_EXTENSION) == 0) {
            write_message(sockfd_to_send, "text/javascript");
        } else if (strcmp(extension, CSS_EXTENSION) == 0) {
            write_message(sockfd_to_send, "text/css");
        // If there is a '.' character found in the file path, but it's either a file extension not part of the four
        // or part of something else in the file path which is not a file extension (which we don't care about).
        } else {
            write_message(sockfd_to_send, "application/octet-stream");
        }
    // If there is no '.' character found which means no file extension.
    } else {
        write_message(sockfd_to_send, "application/octet-stream");
    }
}
