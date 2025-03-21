#ifndef STRING_FILTERING_H
#define STRING_FILTERING_H

#include <linux/types.h>  

extern int strings_exclusions_are_empty();
extern int str_entry_is_excluded(const char *entry);
extern bool is_numeric(const char *str);
extern bool is_mod_directory(unsigned int fd);

#endif // STRING_FILTERING_H

