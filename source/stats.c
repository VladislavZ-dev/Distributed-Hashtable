/* SD-33
 * Luís Filipe Pereira dos Santos 58437
 * Vladislav Zavgorodnii 59783
 * Denis Bahnari 59878
 */

#include <stdio.h>

#include "../include/stats.h"

int stats_complete(struct statistics_t *stats, int clients_on, int ops_num, long time) {
    stats->online_clients = clients_on;
    stats->operations = ops_num;
    stats->time = time;
}

int print_stats(struct statistics_t *stats) {
    printf("Server Statistics:\n");
    printf("  Number of operations processed: %d\n", stats->operations);
    printf("  Time spent by table operations: %ld ms\n", stats->time);
    printf("  Number of online clients: %d\n", stats->online_clients);
}

long get_operation_time_stamp() {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    return current_time.tv_usec;
}