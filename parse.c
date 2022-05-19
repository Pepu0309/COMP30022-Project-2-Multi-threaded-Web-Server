#include "parse.h"
// Takes the request buffer read in from serve_connection and a pointer to the request_path string and then extracts
// the request_path and places it into the request_path string variable. Also does checks to make sure that the format
// of the HTTP request is appropriate. Return true if all checks are passed, return false if there is an issue
// with the request.
bool parse_request_path(char *request_buffer, char **request_path) {
    // Note: From https://man7.org/linux/man-pages/man3/strtok_r.3.html, if the delimiter isn't found, then strtok will
    // scan forward until the null terminator byte, so it will return the whole string. This will then be caught
    // by the checks below. This function should only accept requests which are in the form "GET path HTTP/1.0\r\n\r\n"
    // and indicate an error in any other case.

    // Declare pointers used by strtok_r to internally store where to continue from between successive calls
    // on the same string. https://man7.org/linux/man-pages/man3/strtok_r.3.html
    char *buffer_saveptr;
    char *request_line_saveptr;

    // If something has gone wrong with getting the request line, then we return false to indicate unsuccessful
    // parsing of the request path as well. This also catches the case where a request is an empty string "" as
    // strtok will give NULL back.
    char *request_line = strtok_r(request_buffer, "\r\n", &buffer_saveptr);
    if(request_line == NULL) {
        return false;
    }
    size_t request_line_size = strlen(request_line);

    // Consume the GET which is the first token of strtok_r. If it's not GET then this function returns false.
    char *HTTP_method = strtok_r(request_line, " ", &request_line_saveptr);
    if(HTTP_method == NULL) {
        return false;
    }
    if(strcmp(HTTP_method, GET_REQUEST) != SAME_STRING) {
        return false;
    }

    // Call strtok_r again using NULL as first argument to get the second token which is the supposed file path from
    // the request and return it. If this is not a valid file path (like say this was the method or the protocol
    // version instead), this issue will be caught by the checks later in the program.
    *request_path = strtok_r(NULL, " ", &request_line_saveptr);
    if(*request_path == NULL) {
        return false;
    }

    // Call strtok_r once again to get the HTTP protocol version of the request. If it's not HTTP/1.0, then this
    // function returns false as well.
    char *req_protocol_version = strtok_r(NULL, " ", &request_line_saveptr);
    if(req_protocol_version == NULL) {
        return false;
    }
    if(strcmp(req_protocol_version, PROTOCOL_VER) != SAME_STRING) {
        return false;
    }

    // At this point, we know that the protocol version field is not empty, but we have to make sure that nothing else
    // comes between the protocol version "HTTP/1.0" and "\r\n", the "\r\n" character was stripped away by the first
    // strtok_r in order to get the request line. Call strlen on all the 3 tokens and add 2 more spaces
    // (to count the space delimiters that were disposed of) and compare that to the strlen of request_line saved at
    // the start. If the combined string length of the tokens is less than that of the request line,
    // that means there was something between the protocol version and "\r\n" and the request is invalid.
    size_t combined_token_size = strlen(HTTP_method) + strlen(*request_path) +
            strlen(req_protocol_version) + TWO_SPACES;

    if(combined_token_size < request_line_size) {
        return false;
    }

    // Otherwise, at this point, everything is fine, so we return true
    return true;
}

// Function which calls parse_request_path to obtain the request file path and then form the absolute file path
// using the web root path passed in as a command line argument and the request file path. Returns true if no issues
// are encountered when doing so; false otherwise.
bool get_file_path(char **file_path, char *web_path_root, char *request_buffer) {
    char *request_path;

    // If nothing has gone wrong in parsing the request path, then also check that the request_path does not contain
    // any escape components before creating the full file path.
    if (parse_request_path(request_buffer, &request_path) && web_path_root != NULL) {
        if(check_escape_request_path(request_path)) {
            return false;
        }
        // Nothing is wrong with the request_path.
        size_t file_path_length = strlen(web_path_root) + strlen(request_path) + NULL_TERMINATOR_SPACE;

        *file_path = (char *) malloc (file_path_length * sizeof(char));

        /* Concatenate both the web_path_root and request_path into a new string variable. I was looking for a way
           concatenate them into a new string variable with enough space instead of just concatenating request_path to
           web_path_root and came across this code from a stackoverflow post.
           https://stackoverflow.com/questions/8465006/how-do-i-concatenate-two-strings-in-c
           and used it as reference. While the rest of the code is fairly similar, I wrote it before coming across the
           post. */

        // At this point, web_path_root has been verified to not be NULL (through the check above) and request_path
        // has been checked in parse_request_path.
        // Copy web_path_root to file_path first since it has enough space to store both strings.
        strcpy(*file_path, web_path_root);
        //Then concatenate the request_path to file_path which should already contain web_path_root.
        strcat(*file_path, request_path);
        return true;
    // If something has gone wrong, then we indicate that we were unable to successfully create an
    // absolute file path.
    } else {
        return false;
    }
}

// Function which checks whether there is an escape component within the file path. Returns true if there is; false
// otherwise.
bool check_escape_request_path(char *request_path) {
    // Check that the file path is not NULL. It shouldn't be by this point, but nothing wrong with checking again.
    if(request_path != NULL) {
        // Check if the file path contains "/../" at any point. strstr() returns NULL when there is no occurrences
        // of the specified substring in the string. https://man7.org/linux/man-pages/man3/strstr.3.html
        if(strstr(request_path, "/../") != NULL) {
            return true;
        }
        // Use pointer arithmetic on file_path to get the last 3 characters and check if it's "/.." which means that
        // it's an escape component. Before that, also check that the length of the file path has more than 3
        // characters (strlen does not count null terminator character '\0') so the program doesn't access memory
        // addresses illegally.
        if(strlen(request_path) >= 3) {
            char *last_3_char = request_path + strlen(request_path) - 3;
            if(strcmp(last_3_char, "/..") == SAME_STRING) {
                return true;
            }
        }
    }
    return false;
}

