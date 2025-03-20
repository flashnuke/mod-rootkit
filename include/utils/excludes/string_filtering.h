#ifndef STRING_FILTERING_H
#define STRING_FILTERING_H

#include <linux/types.h>  

extern int str_entry_is_excluded(const char *entry);
extern bool is_numeric(const char *str);

#endif // STRING_FILTERING_H

