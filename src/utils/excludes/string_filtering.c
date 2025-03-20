#include <linux/string.h>

#include "utils/excludes/string_filtering.h"

int str_entry_is_excluded(const char *entry) {
    char excludes[] = STRING_EXCLUDES; // copy to a mutable array
    char *token;
    char *temp_excludes = excludes; // `strsep` modifies the original string

    while ((token = strsep(&temp_excludes, ",")) != NULL) {
        if (strstr(entry, token)) {
            return 1;
        }
    }
    return 0;
}

// function to check if a string contains only numeric characters
bool is_numeric(const char *str) {
    int i;
    if (!str || !str[0])
        return false;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9')
            return false;
    }
    return true;
}

