#ifndef NET_FILTERING_H
#define NET_FILTERING_H

#include <linux/in.h>
#include <linux/inet.h>
#include <linux/string.h>

extern int net_exclusions_are_empty();
extern void ip_to_hex(const char *ip_str, char *hex_buf);
extern void port_to_hex(unsigned int port, char *hex_buf);
extern int should_exclude_line(const char *line);
extern char *filter_netstat_lines(const char *buf, size_t *new_len);

#endif // NET_FILTERING_H

