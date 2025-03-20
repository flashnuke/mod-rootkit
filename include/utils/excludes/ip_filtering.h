#ifndef IP_FILTERING_H
#define IP_FILTERING_H

#include <linux/in.h>
#include <linux/inet.h>
#include <linux/string.h>

extern void ip_to_hex(const char *ip_str, char *hex_buf);
extern void port_to_hex(unsigned int port, char *hex_buf);
extern int should_exclude_line(const char *line);
extern char *filter_netstat_lines(const char *buf, size_t *new_len);

#endif // IP_FILTERING_H

