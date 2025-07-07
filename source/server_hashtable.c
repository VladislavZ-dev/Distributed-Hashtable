/* SD-33
 * Luís Filipe Pereira dos Santos 58437
 * Vladislav Zavgorodnii 59783
 * Denis Bahnari 59878
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "../include/server_network.h"
#include "../include/server_skeleton.h"

struct table_t *table;
int listen_socket;

void handle_sigint(int signum) {
    server_network_close(listen_socket);
    server_skeleton_destroy(table);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Used %s <port> <num_lists>\n", argv[0]);
        return -1;
    }

    short port = atoi(argv[1]);
    int n_lists = atoi(argv[2]);

    table = server_skeleton_init(n_lists);
    if (table == NULL) {
        perror("Error initializing table\n");
        return -1;
    }

    listen_socket = server_network_init(port);
    if (listen_socket == -1) {
        fprintf(stderr, "Error initializing listen socket\n");
        server_skeleton_destroy(table);
    }

    signal(SIGINT, handle_sigint);

    printf("Server is up and waiting connection from port %d...\n", port);

    if (network_main_loop(listen_socket, table) == -1) {
        fprintf(stderr, "Error in the main network loop\n");
        server_network_close(listen_socket);
        server_skeleton_destroy(table);
        return -1;
    }

    server_network_close(listen_socket);
    server_skeleton_destroy(table);
    return 0;
}