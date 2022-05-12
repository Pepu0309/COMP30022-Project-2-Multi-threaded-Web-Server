#include "memory.h"

void string_realloc_check(char **string_ptr, int *cur_strlen, int *cur_max_strlen) {
    if(*cur_strlen >= *cur_max_strlen) {
        *cur_max_strlen *= DOUBLE_CUR_SPACE;
        *string_ptr = (char *) realloc (*string_ptr, (*cur_max_strlen) * sizeof(char));
        assert(*string_ptr != NULL);
    }
}

