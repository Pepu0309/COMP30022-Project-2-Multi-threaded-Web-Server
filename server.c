// A simple server in the internet domain using TCP
// The port number is passed as an argument
// To compile: gcc server.c -o server
// Reference: Beej's networking guide, man pages

// The code in this project was built off of server.c provided as part of COMP30023 Week 9 Practical.
#include "server.h"

int main(int argc, char** argv) {
	int sockfd, newsockfd, re, s;
	struct addrinfo hints, *res, *p;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;

	if (argc < 4) {
		fprintf(stderr, "ERROR, not enough arguments provided.\n");
		exit(EXIT_FAILURE);
	}

    char *web_root_path = argv[3];

	// Create address we're going to listen on (with given port number)
	memset(&hints, 0, sizeof hints);

    if(strcmp(argv[1], IPV4_ARG) == SAME_STRING) {
        hints.ai_family = AF_INET; // IPv4
    } else if (strcmp(argv[1], IPV6_ARG) == SAME_STRING) {
        hints.ai_family = AF_INET6; // IPv6
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
    for (p = res; p != NULL; p = p->ai_next) {
        // hints.ai_family contains the IP address type that we want (AF_INET or AF_INET6). Check that the current
        // address in this node of the linked list corresponds to the address family stored in hints.ai_family.
        if (p->ai_family == hints.ai_family) {
            // We attempt to create a socket from this address. If socket creation was successful, we can use
            // this socket, so we break out of the loop. Otherwise, we keep trying with remaining IPv6 addresses
            // if there are any.
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) >= 0) {
                break;
            }
        }
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

    // Listen on socket - means we're ready to accept connections,
    // incoming connection requests will be queued, man 3 listen
    if (listen(sockfd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(true) {
        // Accept a connection - blocks until a connection is ready to be accepted
        // Get back a new file descriptor to communicate on
        client_addr_size = sizeof client_addr;
        newsockfd =
                accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_size);
        if (newsockfd < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Create a struct that contains the arguments needed to run the serve_connection function as per linux
        // manual and Ed thread #845 https://edstem.org/au/courses/7916/discussion/857869.
        serve_connection_args_t *serve_connection_args =
                (serve_connection_args_t *)malloc(sizeof (serve_connection_args_t));
        serve_connection_args->newsockfd = newsockfd;
        serve_connection_args->web_root_path = web_root_path;

        // Create a pthread_t variable which is used to identify the thread.
        // https://man7.org/linux/man-pages/man3/pthread_create.3.html
        pthread_t thread_id;

        pthread_create(&thread_id, NULL, serve_connection, (void *)serve_connection_args);
    }
    close(sockfd);
	return 0;
}
// Function that is passed into pthread_create. This function takes a struct which contains the socket the thread
// is supposed to serve as well as the web root path. It repeatedly reads packets from the socket and places it in a
// buffer until the request ends. After reading the request, it then calls helper functions to send an appropriate
// HTTP response.
void *serve_connection(void *serve_connection_args) {
    int n, bytes_read_so_far = 0;
    // Use calloc to initialise the buffer so strstr can be called on it.
    char *buffer = (char *) calloc ((REQUEST_MAX_BUFFER_SIZE + NULL_TERMINATOR_SPACE), sizeof(char));

    // Type cast the struct containing the arguments for the function and then extract them and store them in variables.
    int newsockfd = ((serve_connection_args_t *)serve_connection_args)->newsockfd;
    char *web_root_path = ((serve_connection_args_t *)serve_connection_args)->web_root_path;

    // Read characters from the connection, then process them until we encounter "\r\n\r\n" which we check using
    // the strstr() function. "\r\n\r\n" means end of HTTP request.
    do {
        // Pass in buffer + bytes_read_so_far to read() which tells read the offset to begin reading at as per
        // https://man7.org/linux/man-pages/man2/read.2.html. In the case of multi-packet request, read() will continue
        // reading from where it left off at before. n is number of characters read
        n = read(newsockfd, buffer + bytes_read_so_far, REQUEST_MAX_BUFFER_SIZE - bytes_read_so_far);
        if (n < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        // Track the bytes read so far into the buffer.
        bytes_read_so_far += n;
    } while(strstr(buffer, "\r\n\r\n") == NULL);

    // Null-terminate string
    buffer[bytes_read_so_far] = '\0';

    char *file_path;
    // If the program successfully creates a file_path, then we continue as usual
    if(get_file_path(&file_path, web_root_path, buffer)) {
        send_http_response(newsockfd, file_path);
        free(file_path);
        // Otherwise, the program will send a generic 404 Not Found response to the socket
    } else {
        write_message(newsockfd, "HTTP/1.0 404 Not Found\r\n\r\n");
    }

    free(serve_connection_args);
    free(buffer);
    close(newsockfd);

    // pthread_create causes memory leaks. Call pthread_detach pthread_self (this thread) in order to automatically
    // free the resources. Idea was initially seen at the stackoverflow post:
    // https://stackoverflow.com/questions/5610677/valgrind-memory-leak-errors-when-using-pthread-create.
    // Then, research into the linux manual was done to obtain full understanding of the context:
    // https://man7.org/linux/man-pages/man3/pthread_detach.3.html and
    // https://man7.org/linux/man-pages/man3/pthread_self.3.html
    pthread_detach(pthread_self());
    return NULL;
}