/* SD-33
 * Luís Filipe Pereira dos Santos 58437
 * Vladislav Zavgorodnii 59783
 * Denis Bahnari 59878
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "../include/message-private.h"

int write_all(int sock, void *buf, int len) {
    int buf_size = len;
    char *buf_pointer = (char *) buf;

    while (len > 0) {
        int written = write(sock, buf_pointer, len);

        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("failed write all");
            return -1;
        }
        if (written == 0) {
            return 0;
        }
        buf_pointer += written;
        len -= written;
    }
    return buf_size; 
}


int read_all(int sock, void *buf, int len) {
    int buf_size = len;
    char *buf_pointer = (char *) buf;

    while (len > 0) {
        int reading = read(sock, buf_pointer, len);

        if (reading < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("failed to read all");
            return -1;
        }
        if (reading == 0) {
            return 0;
        }
        buf_pointer += reading;
        len -= reading;
    }

    return buf_size; 
}