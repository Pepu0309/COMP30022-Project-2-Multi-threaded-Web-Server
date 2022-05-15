#include "parse.h"

bool parse_request_path(char *request_buffer, char **request_path) {
    // Note: From https://man7.org/linux/man-pages/man3/strtok_r.3.html, if the delimiter isn't found, then strtok will
    // scan forward until the null terminator byte, so it will return the whole string. This will then be caught
    // by the checks below. This function should only accept requests which are in the form "GET path HTTP/1.0\r\n\r\n"
    // and indicate an error in any other case.

    // If something has gone wrong with getting the request line, then we return false to indicate unsuccessful
    // parsing of the request path as well. This also catches the case where a request is an empty string "" as
    // strtok will give NULL back.
    char *request_line = strtok(request_buffer, "\r\n");
    if(request_line == NULL) {
        return false;
    }

    // Consume the GET which is the first token of strtok. If it's not GET then this function returns false
    char *HTTP_method = strtok(request_line, " ");
    if(strcmp(HTTP_method, GET_REQUEST) != 0) {
        return false;
    }

    // Call strtok again using NULL as first argument to get the second token which is the supposed file path from the
    // request and return it. If this is not a valid file path (like say this was the method or the protocol version
    // instead), this issue will be caught by the checks later in the program.
    *request_path = strtok(NULL, " ");
    if(*request_path == NULL) {
        return false;
    }

    // Call strtok once again to get the HTTP protocol version of the request. If it's not HTTP/1.0, then this
    // function returns false as well.
    char *req_protocol_version = strtok(NULL, " ");
    if(strcmp(req_protocol_version, PROTOCOL_VER) != 0) {
        return false;
    }

    // Otherwise, at this point, everything is fine, so we return true
    return true;
}

bool get_file_path(char **file_path, char *web_path_root, char *request_buffer) {
    char *request_path;

    // If nothing has gone wrong in parsing the request path, then we continue with creating the absolute file path.
    if (parse_request_path(request_buffer, &request_path)) {
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
        return true;
        // If something has gone wrong, then we indicate that we were unable to successfully create an absolute file path.
    } else {
        return false;
    }
}

