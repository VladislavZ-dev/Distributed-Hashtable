/* SD-33
 * Luís Filipe Pereira dos Santos 58437
 * Vladislav Zavgorodnii 59783
 * Denis Bahnari 59878
 */

#ifndef _STATS_H
#define _STATS_H

#include <sys/time.h>

struct statistics_t {
    int operations;
    long time;
    int online_clients;
};

/* Completa a estrutura statistics_t com os valores dos stats do servidor
*/
int stats_complete(struct statistics_t *stats, int clients_on, int ops_num, long time);

/* Realiza o print das estatisticas do servidor
*/
int print_stats(struct statistics_t *stats);


/* Devolve o instante do tempo, em ms, quando é chamada
*/
long get_operation_time_stamp();

#endif 