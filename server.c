// A simple server in the internet domain using TCP
// The port number is passed as an argument
// To compile: gcc server.c -o server
// Reference: Beej's networking guide, man pages

// The code in this project was built off of server.c provided as part of COMP30023 Week 9 Practical.
#include "server.h"

int main(int argc, char** argv) {
	int sockfd, newsockfd, n, re, s;
	char buffer[REQUEST_MAX_BUFFER_SIZE + NULL_TERMINATOR_SPACE];
	struct addrinfo hints, *res, *p;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;

	if (argc < 4) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(EXIT_FAILURE);
	}

    char *web_path_root = argv[3];
    bool mode_IPv6 = false;

	// Create address we're going to listen on (with given port number)
	memset(&hints, 0, sizeof hints);

    if(strcmp(argv[1], IPV4_ARG) == 0) {
        hints.ai_family = AF_INET; // IPv4
    } else if (strcmp(argv[1], IPV6_ARG) == 0) {
        hints.ai_family = AF_INET6; // IPv6
        // Set the mode_IPv6 flag to be true so the program knows that it's using IPv6.
        mode_IPv6 = true;
    }
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept

	// node (NULL means any interface), service (port), hints, res. Note: argv[2] is the port_number
	s = getaddrinfo(NULL, argv[2], &hints, &res);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}
    // The code used in the if statement was referenced and modified from COMP30023 Week 8 Lecture 2 Lecture Slide 13
    // "Create IPv6 Socket". getaddrinfo returns multiple addresses in a linked list as mentioned in COMP30023
    // Week 8 Lecture 2. Hence, if we want a IPv6 address, we need to use a for loop to step through the
    // linked list returned (res) and find a valid IPv6 address to use to create a socket.
    if(mode_IPv6) {
        for(p = res; p != NULL; p = p->ai_next) {
            // Check if the address family of the address is IPv6. If it is then we proceed to try to create a socket
            // using this address in the linked list.
            if(p->ai_family == AF_INET6) {
                // We attempt to create a socket from this address. If socket creation was successful, we can use
                // this socket, so we break out of the loop. Otherwise, we keep trying with remaining IPv6 addresses
                // if there are any.
                if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) >= 0) {
                    break;
                }
            }
        }
    // Otherwise, we don't care about IPv6 and create a socket as normal.
    } else {
        // Create socket
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    }

    // If no sockets were successfully created either from the loop to create an IPv6 socket or the socket call() from
    // the else statement to create a normal IPv4 socket, then there was an error and the program exits with failure.
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
        // If the program successfully creates a file_path, then we continue as usual
        if(get_file_path(&file_path, web_path_root, buffer)) {
            send_http_response(newsockfd, file_path);
            free(file_path);
        // Otherwise, the program will send a generic 404 Not Found response to the socket
        } else {
            write_message(newsockfd, "HTTP/1.0 404 Not Found\r\n\r\n");
        }

        close(newsockfd);
    }
    close(sockfd);
	return 0;
}

